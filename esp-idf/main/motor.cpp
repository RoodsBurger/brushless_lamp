// Phase 14c — position control via SimpleFOC *velocity* mode + an app-layer P controller.
// Rationale: M2-knob's silent tuning is in velocity mode (PID_velocity P=0.2 I=20,
// LPF_velocity.Tf=0.02, motion_downsample=5). SimpleFOC's angle mode re-tunes the
// inner loop and tends to be louder on the C6. So we keep the velocity loop that's
// known-quiet and add our own cascade outside:
//
//     position_error = target_angle - shaft_angle
//     desired_vel    = clamp(P_POSITION × position_error, ± MOTION_VELOCITY)
//     commanded_vel  = commanded_vel ±= MOTION_ACCEL·dt   (ramp toward desired_vel)
//     motor.move(commanded_vel)
//
// At the target the error → 0 so desired_vel → 0; the ramp brings commanded_vel to 0
// with a bounded deceleration. No abrupt start/stop, motor runs M2-silent gains.
//
// EN on the DRV8313 is hardwired to 3V3, so motor.disable() just zeroes PWM (grounds
// all phases → no holding torque). We auto-disable IDLE_DISABLE_MS after reaching the
// target so the lamp isn't chattering at rest.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <math.h>

#include "motor.h"
#include "config.h"
#include "pins.h"

static const char *TAG = "motor";

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static volatile float  s_target_angle  = 0.0f;
static volatile bool   s_want_motion   = false;
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;

// At-target tolerance generous on purpose — for a 360-revolution lamp (2262 rad total
// travel) 2 rad is ~0.09 % of the range; well inside "nobody will notice" and it lets
// the motor give up position-chase quickly so idle-disable fires. Rest detection uses
// commanded-vel only; shaft_velocity jitters forever while the velocity PID is active.
static constexpr float ANGLE_AT_TARGET_EPS = 2.0f;
static constexpr float VEL_AT_REST_EPS     = 0.1f;  // rad/s — applied to commanded_vel

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

static float clamp_angle(float rad) {
    if (rad < 0.0f)       return 0.0f;
    if (rad > ANGLE_MAX)  return ANGLE_MAX;
    return rad;
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static void resetDriver() {
    s_motor.disable();
    // Flush PID + cached sensor state so re-enable doesn't kick a step out of stale
    // integral. M2-knob learned this the hard way — same pattern adapted for this loop.
    s_motor.PID_velocity.reset();
    s_motor.shaft_velocity = 0.0f;
    digitalWrite(PIN_NSP, LOW);
    delayMicroseconds(50);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);
    s_motor.enable();
}

#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

static void motor_foc_task(void *) {
    float    commanded_vel     = 0.0f;   // what we pass to motor.move()
    uint32_t last_ramp_us      = 0;
    uint32_t prev_loop_us      = 0;
    uint32_t max_loop_us       = 0;
    uint32_t last_print_ms     = 0;
    uint32_t last_active_ms    = 0;
    float    last_target       = s_target_angle;
    uint32_t iter              = 0;

    while (true) {
        uint32_t now_us = micros();
        if (prev_loop_us != 0) {
            uint32_t dt_loop = now_us - prev_loop_us;
            if (dt_loop > max_loop_us) max_loop_us = dt_loop;
        }
        prev_loop_us = now_us;

        // App-level position loop — physics-based trapezoidal profile:
        //
        //     max_safe_vel = sqrt(2 · MOTION_ACCEL · |pos_err|)
        //
        // is the fastest we can move right now and still decelerate to 0 exactly at
        // the target given our accel limit. Clamping desired_vel to this guarantees
        // no overshoot. A linear P-controller was stopping far too late for the 25
        // rad/s approach velocity (brake distance = 31 rad with accel=10). The direction
        // sign comes from pos_err. A brake-first reversal detector still sits on top so
        // a mid-motion slider flip doesn't try to jump into the new direction before
        // commanded_vel has decelerated.
        float pos_err     = s_target_angle - s_motor.shaft_angle;
        float brake_v     = sqrtf(2.0f * MOTION_ACCEL * fabsf(pos_err));
        float desired_mag = fminf(MOTION_VELOCITY, brake_v);
        float desired_raw = (pos_err >= 0.0f) ? desired_mag : -desired_mag;

        const float MOTION_EPS = 0.5f;   // rad/s — consider this "stopped"
        bool currently_fwd = commanded_vel >  MOTION_EPS;
        bool currently_rev = commanded_vel < -MOTION_EPS;
        bool want_fwd      = desired_raw   >  MOTION_EPS;
        bool want_rev      = desired_raw   < -MOTION_EPS;
        bool reversing     = (want_fwd && currently_rev) || (want_rev && currently_fwd);

        float desired_vel = reversing ? 0.0f : desired_raw;

        float dt = (last_ramp_us == 0) ? 0.0f : (now_us - last_ramp_us) * 1e-6f;
        last_ramp_us = now_us;
        if (dt > 0.0f && dt < 0.1f) {
            float step = MOTION_ACCEL * dt;
            if      (commanded_vel <  desired_vel - step) commanded_vel += step;
            else if (commanded_vel >  desired_vel + step) commanded_vel -= step;
            else                                          commanded_vel  = desired_vel;
        } else {
            commanded_vel = desired_vel;  // first sample or long gap → snap
        }

        // Activity tracking for idle auto-disable: "active" = target just changed, or
        // we're still commanding non-trivial velocity, or we're not at target.
        bool at_target  = fabsf(pos_err) < ANGLE_AT_TARGET_EPS;
        // Don't include shaft_velocity in "at rest" — it jitters indefinitely while
        // the velocity PID is actively holding 0, which blocked idle-disable. cmd_vel
        // is our own ramped output and drops cleanly to 0 when the position error is
        // inside ANGLE_AT_TARGET_EPS.
        bool at_rest    = fabsf(commanded_vel) < VEL_AT_REST_EPS;
        if (s_target_angle != last_target) {
            last_target    = s_target_angle;
            last_active_ms = millis();
        }
        if (!at_target || !at_rest) last_active_ms = millis();

        bool should_enable;
        if (!s_want_motion) {
            should_enable = !at_target;      // still reaching 0
        } else {
            should_enable = !at_target || !at_rest ||
                            (millis() - last_active_ms) < IDLE_DISABLE_MS;
        }

        if (should_enable && !s_motor.enabled) {
            resetDriver();
            commanded_vel  = 0.0f;
            last_active_ms = millis();
        } else if (!should_enable && s_motor.enabled) {
            s_motor.disable();
            commanded_vel = 0.0f;
        }

        s_motor.loopFOC();
        s_motor.move(commanded_vel);

        s_shaft_angle_cached    = s_motor.shaft_angle;
        s_shaft_velocity_cached = s_motor.shaft_velocity;

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            printf("tgt:%ld.%02ld ang:%ld.%02ld vel:%ld.%02ld cmd:%ld.%02ld en:%d wm:%d maxLp:%luus\n",
                   FX_HI(s_target_angle,          100), FX_LO(s_target_angle,          100),
                   FX_HI(s_motor.shaft_angle,     100), FX_LO(s_motor.shaft_angle,     100),
                   FX_HI(s_motor.shaft_velocity,  100), FX_LO(s_motor.shaft_velocity,  100),
                   FX_HI(commanded_vel,           100), FX_LO(commanded_vel,           100),
                   (int)s_motor.enabled,
                   (int)s_want_motion,
                   (unsigned long)max_loop_us);
            max_loop_us = 0;
        }

        // Yield much more often than before — under Matter bursts single iterations can
        // hit 15 ms, so the old every-256 yield let IDLE WDT fire. 32 iters ≈ 32 ms
        // worst case, well under the 5 s WDT.
        if ((++iter & 0x1F) == 0) vTaskDelay(1);
    }
}

void motor_init_and_start() {
    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);

    s_encoder.quadrature = Quadrature::ON;
    s_encoder.pullup     = Pullup::USE_INTERN;
    s_encoder.init();
    s_encoder.enableInterrupts(doMotorA, doMotorB);
    s_motor.linkSensor(&s_encoder);

    s_driver.voltage_power_supply = SUPPLY_VOLTAGE;
    s_driver.voltage_limit        = SUPPLY_VOLTAGE;
    s_driver.pwm_frequency        = PWM_FREQ_HZ;
    printf("driver.init()=%d\n", s_driver.init());
    s_motor.linkDriver(&s_driver);

    // Motor-side tuning = M2-knob exact, the only configuration on this hardware
    // confirmed silent + strong. Position control is layered on top in the task.
    s_motor.phase_resistance     = PHASE_RESISTANCE;
    s_motor.KV_rating            = KV_RATING;
    s_motor.current_limit        = CURRENT_LIMIT;
    s_motor.velocity_limit       = VELOCITY_LIMIT;
    s_motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    s_motor.controller           = MotionControlType::velocity;   // M2-proven
    s_motor.torque_controller    = TorqueControlType::voltage;
    s_motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    s_motor.motion_downsample    = MOTION_DOWNSAMPLE;

    s_motor.PID_velocity.P           = PID_P;
    s_motor.PID_velocity.I           = PID_I;
    s_motor.PID_velocity.D           = PID_D;
    s_motor.PID_velocity.output_ramp = PID_OUTPUT_RAMP;
    s_motor.LPF_velocity.Tf          = LPF_TF;

    printf("motor.init()=%d\n", s_motor.init());

    int foc_ok = s_motor.initFOC();
    printf("motor.initFOC()=%d  zero_elec=%.4f  dir=%d\n",
           foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction);
    if (!foc_ok) printf("FOC init failed\n");

    s_motor.disable();

    xTaskCreate(motor_foc_task, "motor_foc", 8192, nullptr, 5, nullptr);
}

void motor_enable()                         { s_want_motion = true; }
void motor_disable()                        { s_want_motion = false; }
void motor_set_target_angle(float rad)      { s_target_angle = clamp_angle(rad); }
void motor_nudge_target_angle(float d_rad)  { s_target_angle = clamp_angle(s_target_angle + d_rad); }

float motor_get_target_angle()    { return s_target_angle; }
float motor_get_shaft_angle()     { return s_shaft_angle_cached; }
float motor_get_shaft_velocity()  { return s_shaft_velocity_cached; }
