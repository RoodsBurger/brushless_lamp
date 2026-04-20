// Brushless Lamp — closed-loop FOC
// Seeed XIAO ESP32-C6 + SimpleFOC Mini (DRV8313) + 4015 BLDC gimbal + MT6701 ABZ encoder

#include <SimpleFOC.h>

constexpr uint8_t PIN_PWM_A = 18;    // D10 — DRV8313 IN1
constexpr uint8_t PIN_PWM_B = 20;    // D9  — DRV8313 IN2
constexpr uint8_t PIN_PWM_C = 19;    // D8  — DRV8313 IN3
constexpr uint8_t PIN_NSP   = 17;    // D7  — DRV8313 nSLEEP (pulse LOW to clear latched fault)

constexpr uint8_t PIN_ENC_A = 0;     // D0  — MT6701 A
constexpr uint8_t PIN_ENC_B = 1;     // D1  — MT6701 B

constexpr uint32_t PWM_FREQ_HZ = 25000;

constexpr int   POLE_PAIRS     = 11;
constexpr int   ENCODER_CPR    = 1024;
constexpr float SUPPLY_VOLTAGE = 24.0f;

constexpr float PHASE_RESISTANCE    = 5.0f;
constexpr float KV_RATING           = 100.0f;
constexpr float CURRENT_LIMIT       = 0.5f;
constexpr float VELOCITY_LIMIT      = 20.0f;
constexpr float VOLTAGE_SENSOR_ALIGN = 3.0f;

constexpr float PID_P               = 0.2f;
constexpr float PID_I               = 20.0f;
constexpr float PID_D               = 0.0f;
constexpr float PID_OUTPUT_RAMP     = 1000.0f;
constexpr float LPF_TF              = 0.02f;
constexpr int   MOTION_DOWNSAMPLE   = 5;

constexpr uint32_t PRINT_INTERVAL_MS = 1000;

constexpr uint32_t IDLE_DISABLE_MS    = 300;
constexpr float    IDLE_TARGET_THRESH = 0.01f;

constexpr float    STALL_VEL_THRESHOLD = 0.2f;
constexpr uint32_t STALL_TIMEOUT_MS    = 500;
constexpr uint32_t STALL_WARMUP_MS     = 800;

constexpr float    TARGET_ACCEL_DEFAULT = 10.0f;   // rad/s², tune via 'A' command

BLDCMotor       motor   = BLDCMotor(POLE_PAIRS);
BLDCDriver3PWM  driver  = BLDCDriver3PWM(PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
Encoder         encoder = Encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);
Commander       command = Commander(Serial);

void IRAM_ATTR doA() { encoder.handleA(); }
void IRAM_ATTR doB() { encoder.handleB(); }

float target        = 0.0f;                 // user-commanded velocity, rad/s
float ramped_target = 0.0f;                 // accel-limited velocity fed to motor.move
float accel         = TARGET_ACCEL_DEFAULT; // rad/s² acceleration cap

// Move ramped_target toward target at 'accel' rad/s².
void updateRamp() {
    static uint32_t last_us = 0;
    uint32_t now = micros();
    float dt = (now - last_us) * 1e-6f;
    last_us = now;
    if (dt <= 0 || dt > 0.1f) { ramped_target = target; return; }
    float step = accel * dt;
    if      (ramped_target <  target - step) ramped_target += step;
    else if (ramped_target >  target + step) ramped_target -= step;
    else                                     ramped_target  = target;
}

// Zero PID + filtered velocity so the re-enabled driver doesn't fire a voltage spike.
void flushControlState() {
    motor.PID_velocity.reset();
    motor.shaft_velocity = 0.0f;
    ramped_target        = 0.0f;
}

// Pulse nSLEEP low ≥30 µs to clear a latched DRV8313 OCP/TSD fault.
void resetDriver() {
    motor.disable();
    flushControlState();
    digitalWrite(PIN_NSP, LOW);
    delayMicroseconds(50);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);
    motor.enable();
}

// Emergency stop: kill target, disable driver, zero control state.
void haltMotor() {
    target = 0.0f;
    motor.disable();
    flushControlState();
}

void doTarget (char* arg) { command.scalar(&target, arg); }
void doCurrent(char* arg) { command.scalar(&motor.current_limit, arg); }
void doP      (char* arg) { command.scalar(&motor.PID_velocity.P, arg); }
void doI      (char* arg) { command.scalar(&motor.PID_velocity.I, arg); }
void doTf     (char* arg) { command.scalar(&motor.LPF_velocity.Tf, arg); }
void doR      (char* arg) { command.scalar(&motor.phase_resistance, arg); }
void doK      (char* arg) { command.scalar(&motor.KV_rating, arg); }
void doM      (char* arg) { float v; command.scalar(&v, arg); motor.motion_downsample = (unsigned int)v; }
void doAccel  (char* arg) { command.scalar(&accel, arg); }
void doReset  (char* arg) { (void)arg; resetDriver(); Serial.println("driver reset"); }

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("=== BrushlessLamp: closed-loop FOC ===");

    pinMode(PIN_NSP, OUTPUT);
    digitalWrite(PIN_NSP, HIGH);
    delay(2);

    encoder.quadrature = Quadrature::ON;
    encoder.pullup     = Pullup::USE_INTERN;
    encoder.init();
    encoder.enableInterrupts(doA, doB);
    motor.linkSensor(&encoder);

    driver.voltage_power_supply = SUPPLY_VOLTAGE;
    driver.voltage_limit        = SUPPLY_VOLTAGE;
    driver.pwm_frequency        = PWM_FREQ_HZ;
    Serial.printf("driver.init()=%d\n", driver.init());
    motor.linkDriver(&driver);

    motor.phase_resistance     = PHASE_RESISTANCE;
    motor.KV_rating            = KV_RATING;
    motor.current_limit        = CURRENT_LIMIT;
    motor.velocity_limit       = VELOCITY_LIMIT;
    motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    motor.controller           = MotionControlType::velocity;
    motor.torque_controller    = TorqueControlType::voltage;
    motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    motor.motion_downsample    = MOTION_DOWNSAMPLE;

    motor.PID_velocity.P           = PID_P;
    motor.PID_velocity.I           = PID_I;
    motor.PID_velocity.D           = PID_D;
    motor.PID_velocity.output_ramp = PID_OUTPUT_RAMP;
    motor.LPF_velocity.Tf          = LPF_TF;

    motor.useMonitoring(Serial);
    Serial.printf("motor.init()=%d\n", motor.init());

    int foc_ok = motor.initFOC();
    Serial.printf("motor.initFOC()=%d  zero_elec=%.4f  dir=%d\n",
                  foc_ok, motor.zero_electric_angle, (int)motor.sensor_direction);
    if (!foc_ok) Serial.println("FOC init failed");

    command.add('T', doTarget,  "target velocity");
    command.add('C', doCurrent, "current limit");
    command.add('P', doP,       "PID P");
    command.add('I', doI,       "PID I");
    command.add('F', doTf,      "LPF Tf");
    command.add('R', doR,       "phase R");
    command.add('K', doK,       "KV rating");
    command.add('M', doM,       "motion downsample");
    command.add('A', doAccel,   "velocity accel rad/s^2");
    command.add('X', doReset,   "reset driver (clear latched fault)");
    Serial.println("Ready.");
}

// Fixed-point printf args — C6 has no FPU and %.2f costs ~300 µs per arg.
#define FX_HI(v, scale) ((long)((int32_t)((v) * (scale)) / (scale)))
#define FX_LO(v, scale) ((long)((((int32_t)((v) * (scale))) < 0 ? -((int32_t)((v) * (scale))) : ((int32_t)((v) * (scale)))) % (scale)))

// Never returns — loopTask preemption on single-core ESP32 variants would stall FOC ~4 ms every 2 s. See README.
void loop() {
    static uint32_t prev_loop_us = 0;
    static uint32_t max_loop_us  = 0;
    static uint32_t last_active_ms = 0;
    static uint32_t stall_start_ms = 0;
    static uint32_t enable_time_ms = 0;
    static uint32_t last_print_ms  = 0;

    while (true) {
        uint32_t now_us = micros();
        if (prev_loop_us != 0) {
            uint32_t dt = now_us - prev_loop_us;
            if (dt > max_loop_us) max_loop_us = dt;
        }
        prev_loop_us = now_us;

        command.run();

        bool want_motion  = fabs(target) > IDLE_TARGET_THRESH;
        bool ramping_down = fabs(ramped_target) > IDLE_TARGET_THRESH;

        if (want_motion) {
            last_active_ms = millis();

            if (!motor.enabled) {
                // Full nSLEEP reset on wake clears any latched fault before commutation resumes.
                resetDriver();
                enable_time_ms = millis();
                stall_start_ms = 0;
            }

            // Skip stall detection during the accelerate-from-stop warmup.
            bool in_warmup = (millis() - enable_time_ms) < STALL_WARMUP_MS;
            if (!in_warmup && fabs(motor.shaft_velocity) < STALL_VEL_THRESHOLD) {
                if (stall_start_ms == 0) stall_start_ms = millis();
                else if (millis() - stall_start_ms > STALL_TIMEOUT_MS) {
                    Serial.printf("stall at ang=%ld.%ld while commanding T=%ld.%02ld — halted. "
                                  "Send T with opposite sign to back off.\n",
                                  FX_HI(motor.shaft_angle, 10), FX_LO(motor.shaft_angle, 10),
                                  FX_HI(target, 100), FX_LO(target, 100));
                    haltMotor();
                    stall_start_ms = 0;
                }
            } else {
                stall_start_ms = 0;
            }
        } else {
            stall_start_ms = 0;
            // Keep the idle timer fresh while the ramp is still decelerating.
            if (ramping_down) last_active_ms = millis();
            if (motor.enabled && !ramping_down && millis() - last_active_ms > IDLE_DISABLE_MS) {
                motor.disable();
            }
        }

        updateRamp();
        motor.loopFOC();
        motor.move(ramped_target);

        if (millis() - last_print_ms >= PRINT_INTERVAL_MS) {
            last_print_ms = millis();
            // Skip the write if the TX ring can't take the whole line without blocking.
            if (Serial.availableForWrite() >= 96) {
                Serial.printf("tgt:%ld.%02ld ramp:%ld.%02ld vel:%ld.%02ld ang:%ld.%ld en:%d maxLp:%luus\n",
                              FX_HI(target,         100), FX_LO(target,         100),
                              FX_HI(ramped_target,  100), FX_LO(ramped_target,  100),
                              FX_HI(motor.shaft_velocity, 100), FX_LO(motor.shaft_velocity, 100),
                              FX_HI(motor.shaft_angle,    10),  FX_LO(motor.shaft_angle,    10),
                              (int)motor.enabled,
                              (unsigned long)max_loop_us);
            }
            max_loop_us = 0;
        }
    }
}
