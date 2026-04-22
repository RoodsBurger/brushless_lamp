// Phase 14a — motor-only validation. Ports the /tmp/m2_knob/BrushlessLamp/BrushlessLamp.ino
// loop() body verbatim into a dedicated FreeRTOS task. The Arduino runtime is up (via
// initArduino() in app_main), so SimpleFOC 2.4.0's ESP32 MCPWM driver picks its own
// hardware path without any porting work.
//
// The one structural difference vs the Arduino sketch: Arduino wraps the spin-loop in a
// 2 s-preempted task, so the sketch puts `while(true){...}` inside loop(). Here we own
// the task; we just need to feed the IDLE watchdog with a periodic vTaskDelay(1) so
// arduino-as-component's tick rate stays healthy under load (Phase 12 lesson).

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

static volatile float  s_target        = 0.0f;
static volatile float  s_shaft_velocity_cached = 0.0f;
static volatile float  s_shaft_angle_cached    = 0.0f;
static float           s_ramped_target = 0.0f;
static float           s_accel         = TARGET_ACCEL_DEFAULT;

static void IRAM_ATTR doMotorA() { s_encoder.handleA(); }
static void IRAM_ATTR doMotorB() { s_encoder.handleB(); }

// Arduino's noInterrupts/interrupts macros are not always defined on IDF builds; use the
// global interrupt toggles from the ESP32 HAL (same effect — portable across IDF targets).
#define NOINT_BEGIN portDISABLE_INTERRUPTS()
#define NOINT_END   portENABLE_INTERRUPTS()

static void updateRamp() {
    static uint32_t last_us = 0;
    uint32_t now = micros();
    float dt = (now - last_us) * 1e-6f;
    last_us = now;
    if (dt <= 0 || dt > 0.1f) { s_ramped_target = s_target; return; }
    float step = s_accel * dt;
    if      (s_ramped_target <  s_target - step) s_ramped_target += step;
    else if (s_ramped_target >  s_target + step) s_ramped_target -= step;
    else                                         s_ramped_target  = s_target;
}

static void flushControlState() {
    s_motor.PID_velocity.reset();
    s_motor.shaft_velocity = 0.0f;
    s_ramped_target        = 0.0f;
}

static void resetDriver() {
    s_motor.disable();
    flushControlState();
    digitalWrite(PIN_NSP, LOW);
    delayMicroseconds(50);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);
    s_motor.enable();
}

static void haltMotor() {
    s_target = 0.0f;
    s_motor.disable();
    flushControlState();
}

// Fixed-point formatters to dodge soft-float printf — ESP32-C6 has no FPU and each %.2f
// costs ~300 us, enough to blow the inner-loop budget.
#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

static void motor_foc_task(void *) {
    uint32_t prev_loop_us   = 0;
    uint32_t max_loop_us    = 0;
    uint32_t last_active_ms = 0;
    uint32_t stall_start_ms = 0;
    uint32_t enable_time_ms = 0;
    uint32_t last_print_ms  = 0;
    uint32_t iter           = 0;

    while (true) {
        uint32_t now_us = micros();
        if (prev_loop_us != 0) {
            uint32_t dt = now_us - prev_loop_us;
            if (dt > max_loop_us) max_loop_us = dt;
        }
        prev_loop_us = now_us;

        bool want_motion  = fabsf(s_target)        > IDLE_TARGET_THRESH;
        bool ramping_down = fabsf(s_ramped_target) > IDLE_TARGET_THRESH;

        if (want_motion) {
            last_active_ms = millis();
            if (!s_motor.enabled) {
                resetDriver();
                enable_time_ms = millis();
                stall_start_ms = 0;
            }
            bool in_warmup = (millis() - enable_time_ms) < STALL_WARMUP_MS;
            if (!in_warmup && fabsf(s_motor.shaft_velocity) < STALL_VEL_THRESHOLD) {
                if (stall_start_ms == 0) stall_start_ms = millis();
                else if (millis() - stall_start_ms > STALL_TIMEOUT_MS) {
                    ESP_LOGW(TAG, "stall at ang=%ld.%ld while T=%ld.%02ld — halted",
                             FX_HI(s_motor.shaft_angle, 10), FX_LO(s_motor.shaft_angle, 10),
                             FX_HI(s_target,            100), FX_LO(s_target,           100));
                    haltMotor();
                    stall_start_ms = 0;
                }
            } else {
                stall_start_ms = 0;
            }
        } else {
            stall_start_ms = 0;
            if (ramping_down) last_active_ms = millis();
            if (s_motor.enabled && !ramping_down && millis() - last_active_ms > IDLE_DISABLE_MS) {
                s_motor.disable();
            }
        }

        updateRamp();
        s_motor.loopFOC();
        s_motor.move(s_ramped_target);

        s_shaft_velocity_cached = s_motor.shaft_velocity;
        s_shaft_angle_cached    = s_motor.shaft_angle;

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            // Use printf (IDF stdout → USB_SERIAL_JTAG) rather than Serial.printf — Arduino
            // Serial on C6 defaults to UART0 on GPIO 16/17 where our nSLEEP lives, and the
            // output never reaches the host. IDF's stdout is wired to USB_SERIAL_JTAG so
            // the format matches M2-knob's reference log byte-for-byte.
            printf("tgt:%ld.%02ld ramp:%ld.%02ld vel:%ld.%02ld ang:%ld.%ld en:%d maxLp:%luus\n",
                   FX_HI(s_target,                  100), FX_LO(s_target,                  100),
                   FX_HI(s_ramped_target,           100), FX_LO(s_ramped_target,           100),
                   FX_HI(s_motor.shaft_velocity,    100), FX_LO(s_motor.shaft_velocity,    100),
                   FX_HI(s_motor.shaft_angle,        10), FX_LO(s_motor.shaft_angle,        10),
                   (int)s_motor.enabled,
                   (unsigned long)max_loop_us);
            max_loop_us = 0;
        }

        // Feed IDLE-WDT under the 1 kHz FreeRTOS tick — once per 256 iters is enough.
        if ((++iter & 0xFF) == 0) vTaskDelay(1);
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

    // Skip useMonitoring — Arduino Serial is UART0 on this board and SimpleFOC's monitor
    // only adds noise relative to our own 1-Hz tgt/ramp/vel line. printf goes to the
    // working USB_SERIAL_JTAG console either way.
    printf("motor.init()=%d\n", s_motor.init());

    int foc_ok = s_motor.initFOC();
    printf("motor.initFOC()=%d  zero_elec=%.4f  dir=%d\n",
           foc_ok, s_motor.zero_electric_angle, (int)s_motor.sensor_direction);
    if (!foc_ok) printf("FOC init failed\n");

    // Priority 5: above CHIP task (2) so Matter can't starve us in 14c, below TCP/IP (18)
    // so the network stack's still responsive. Stack 8 kB covers SimpleFOC + monitor.
    xTaskCreate(motor_foc_task, "motor_foc", 8192, nullptr, 5, nullptr);
}

void motor_set_target_velocity(float rad_per_sec) {
    if (rad_per_sec >  VELOCITY_LIMIT) rad_per_sec =  VELOCITY_LIMIT;
    if (rad_per_sec < -VELOCITY_LIMIT) rad_per_sec = -VELOCITY_LIMIT;
    s_target = rad_per_sec;
}

float motor_get_target_velocity() { return s_target; }
float motor_get_shaft_velocity()  { return s_shaft_velocity_cached; }
float motor_get_shaft_angle()     { return s_shaft_angle_cached; }
void  motor_stop()                { s_target = 0.0f; }
