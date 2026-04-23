// Phase 14c — position control via SimpleFOC velocity mode (M2-proven quiet) plus an
// app-layer linear+physics profile closing the position loop. Matches M2-knob's idle
// pattern: motor stays enabled while commanded_vel is nonzero or the shaft is moving,
// then disables after IDLE_DISABLE_MS of rest — AND we stop calling motor.move() once
// disabled, so phases stay at 0 (motor.disable() only issues one setPwm(0,0,0); without
// the skip, the next move() re-drives the bridges via the velocity PID).
//
// Position loop:
//     pos_err = target - shaft_angle
//     desired_mag = min( MOTION_VELOCITY,
//                        P_POSITION · |pos_err|,    (linear — smooth asymptote)
//                        sqrt(2 · MOTION_ACCEL · |pos_err|) )  (physics — no overshoot)
//     desired_raw = sign(pos_err) · desired_mag
//     brake-on-reverse: if desired_raw opposes commanded_vel direction, force 0 first
//     commanded_vel = ramp(commanded_vel, desired_vel, MOTION_ACCEL · dt)
//
// At the target the linear term dominates so commanded_vel asymptotically decays to
// zero — no "snap" at any deadband boundary because there isn't one. Disable is time-
// driven (at_rest for IDLE_DISABLE_MS), not position-driven.

#include <Arduino.h>
#include <SimpleFOC.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <math.h>

#include "motor.h"
#include "matter_app.h"
#include "config.h"
#include "pins.h"

static const char *TAG = "motor";

static BLDCMotor       s_motor  (POLE_PAIRS);
static BLDCDriver3PWM  s_driver (PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         s_encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static volatile float  s_target_angle  = 0.0f;
static volatile bool   s_want_motion   = false;   // informational; unused by the loop
static volatile float  s_shaft_angle_cached    = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;
static volatile bool   s_sync_pending  = false;   // knob asked for post-settle Matter push

static constexpr float VEL_AT_REST_EPS = 0.1f;    // rad/s — idle-disable threshold

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

#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

static void motor_foc_task(void *) {
    float    commanded_vel   = 0.0f;
    uint32_t last_ramp_us    = 0;
    uint32_t prev_loop_us    = 0;
    uint32_t max_loop_us     = 0;
    uint32_t last_print_ms   = 0;
    uint32_t last_active_ms  = 0;
    uint32_t iter            = 0;

    while (true) {
        uint32_t now_us = micros();
        if (prev_loop_us != 0) {
            uint32_t dt_loop = now_us - prev_loop_us;
            if (dt_loop > max_loop_us) max_loop_us = dt_loop;
        }
        prev_loop_us = now_us;

        float pos_err = s_target_angle - s_motor.shaft_angle;

        // Combined desired velocity: min(linear, physics, MAX).
        float linear_v    = P_POSITION * fabsf(pos_err);
        float brake_v     = sqrtf(2.0f * MOTION_ACCEL * fabsf(pos_err));
        float desired_mag = fminf(fminf(MOTION_VELOCITY, linear_v), brake_v);
        float desired_raw = (pos_err >= 0.0f) ? desired_mag : -desired_mag;

        // Brake-on-reverse — the only direction-sensitive rule. Same-sign updates
        // flow through the ramp normally; opposite-sign force through zero first.
        const float MOTION_EPS = 0.5f;
        bool currently_fwd = commanded_vel >  MOTION_EPS;
        bool currently_rev = commanded_vel < -MOTION_EPS;
        bool want_fwd      = desired_raw   >  MOTION_EPS;
        bool want_rev      = desired_raw   < -MOTION_EPS;
        bool reversing     = (want_fwd && currently_rev) || (want_rev && currently_fwd);
        float desired_vel  = reversing ? 0.0f : desired_raw;

        float dt = (last_ramp_us == 0) ? 0.0f : (now_us - last_ramp_us) * 1e-6f;
        last_ramp_us = now_us;
        if (dt > 0.0f && dt < 0.1f) {
            float step = MOTION_ACCEL * dt;
            if      (commanded_vel <  desired_vel - step) commanded_vel += step;
            else if (commanded_vel >  desired_vel + step) commanded_vel -= step;
            else                                          commanded_vel  = desired_vel;
        } else {
            commanded_vel = desired_vel;
        }

        // Idle-disable (M2 pattern): software command AND physical shaft both at rest
        // AND we're already at target → start the timer. Target change (pos_err
        // nonzero) immediately counts as active so the wake path doesn't stall for
        // the 10 ms it takes the ramp to lift commanded_vel above VEL_AT_REST_EPS.
        bool at_rest = fabsf(commanded_vel)            < VEL_AT_REST_EPS &&
                       fabsf(s_motor.shaft_velocity)   < VEL_AT_REST_EPS &&
                       fabsf(pos_err)                  < 0.1f;
        if (!at_rest) last_active_ms = millis();
        bool should_enable = (millis() - last_active_ms) < IDLE_DISABLE_MS;

        if (should_enable) {
            if (!s_motor.enabled) s_motor.enable();
            s_motor.loopFOC();
            s_motor.move(commanded_vel);
        } else {
            // Just crossed into idle → cut PWM, flush PID + sensor cache, and if the
            // last motion came from the knob, push the settled angle to Matter now.
            if (s_motor.enabled) {
                s_motor.disable();
                s_motor.PID_velocity.reset();
                s_motor.shaft_velocity = 0.0f;
                commanded_vel = 0.0f;
                if (s_sync_pending) {
                    matter_push_level_from_angle(s_motor.shaft_angle);
                    s_sync_pending = false;
                }
            }
            // Don't call loopFOC/move while disabled — leaving the phases grounded
            // is the whole point of the disable. The encoder ISR still updates
            // shaft_angle in the background, so the next target change will see it.
        }

        s_shaft_angle_cached    = s_motor.shaft_angle;
        s_shaft_velocity_cached = s_motor.shaft_velocity;

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            printf("tgt:%ld.%02ld ang:%ld.%02ld vel:%ld.%02ld cmd:%ld.%02ld en:%d maxLp:%luus\n",
                   FX_HI(s_target_angle,          100), FX_LO(s_target_angle,          100),
                   FX_HI(s_motor.shaft_angle,     100), FX_LO(s_motor.shaft_angle,     100),
                   FX_HI(s_motor.shaft_velocity,  100), FX_LO(s_motor.shaft_velocity,  100),
                   FX_HI(commanded_vel,           100), FX_LO(commanded_vel,           100),
                   (int)s_motor.enabled,
                   (unsigned long)max_loop_us);
            max_loop_us = 0;
        }

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

    // M2-proven velocity-mode tuning (silent, strong). The position loop lives in
    // the task on top of this.
    s_motor.phase_resistance     = PHASE_RESISTANCE;
    s_motor.KV_rating            = KV_RATING;
    s_motor.current_limit        = CURRENT_LIMIT;
    s_motor.velocity_limit       = VELOCITY_LIMIT;
    s_motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    s_motor.controller           = MotionControlType::velocity;
    s_motor.torque_controller    = TorqueControlType::voltage;
    s_motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    s_motor.motion_downsample    = MOTION_DOWNSAMPLE;

    s_motor.PID_velocity.P           = PID_P;
    s_motor.PID_velocity.I           = PID_I;
    s_motor.PID_velocity.D           = PID_D;
    s_motor.PID_velocity.output_ramp = PID_OUTPUT_RAMP;
    s_motor.LPF_velocity.Tf          = LPF_TF;

    s_motor.modulation_centered  = 1;

    printf("motor.init()=%d\n", s_motor.init());

    int foc_ok = s_motor.initFOC();
    printf("motor.initFOC()=%d  zero_elec=%.4f  dir=%d\n",
           foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction);
    if (!foc_ok) printf("FOC init failed\n");

    // Start disabled. First target change wakes it up.
    s_motor.disable();

    xTaskCreate(motor_foc_task, "motor_foc", 8192, nullptr, 5, nullptr);
}

void motor_enable()                         { s_want_motion = true; }
void motor_disable()                        { s_want_motion = false; }
void motor_set_target_angle(float rad)      { s_target_angle = clamp_angle(rad); }
void motor_nudge_target_angle(float d_rad)  { s_target_angle = clamp_angle(s_target_angle + d_rad); }
void motor_request_matter_sync_on_settle()  { s_sync_pending = true; }

float motor_get_target_angle()    { return s_target_angle; }
float motor_get_shaft_angle()     { return s_shaft_angle_cached; }
float motor_get_shaft_velocity()  { return s_shaft_velocity_cached; }
