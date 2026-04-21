#include "motor.h"
#include "leds.h"
#include "pins.h"
#include "config.h"
#include "persistence.h"

#include "esp_simplefoc.h"
#include "sensors/Encoder.h"
#include "driver/gpio.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <atomic>
#include <math.h>

static const char *TAG = "motor";

static BLDCMotor       g_motor   = BLDCMotor(POLE_PAIRS);
static BLDCDriver3PWM  g_driver  = BLDCDriver3PWM(PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         g_encoder = Encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static std::atomic<bool> g_init_done{false};

static std::atomic<float> g_commanded{0.0f};
static std::atomic<float> g_target{0.0f};
static std::atomic<float> g_target_vel{0.0f};
static float              g_accel = ACCEL_DEFAULT;
static float              g_velocity_limit = VELOCITY_PRESETS[DEFAULT_SPEED_IDX];

static void IRAM_ATTR enc_a_isr() { g_encoder.handleA(); }
static void IRAM_ATTR enc_b_isr() { g_encoder.handleB(); }

static gptimer_handle_t g_foc_timer = nullptr;

// Matches Arduino M4's onFocTick(): SVPWM commutation runs at a guaranteed 1 kHz regardless
// of what the app_loop_task is doing under Matter load. Started from app_main.cpp AFTER
// esp_matter::start() so Matter's commissioning storm gets its interrupt resources first.
static bool IRAM_ATTR foc_isr_cb(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *) {
    g_motor.loopFOC();
    return false;
}

void motor_start_foc_timer() {
    if (g_foc_timer) return;
    gptimer_config_t cfg = {};
    cfg.clk_src       = GPTIMER_CLK_SRC_DEFAULT;
    cfg.direction     = GPTIMER_COUNT_UP;
    cfg.resolution_hz = 1000000;
    ESP_ERROR_CHECK(gptimer_new_timer(&cfg, &g_foc_timer));

    gptimer_event_callbacks_t cbs = {};
    cbs.on_alarm = foc_isr_cb;
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(g_foc_timer, &cbs, nullptr));
    ESP_ERROR_CHECK(gptimer_enable(g_foc_timer));

    gptimer_alarm_config_t alarm = {};
    alarm.reload_count               = 0;
    alarm.alarm_count                = 1000000 / FOC_ISR_HZ;
    alarm.flags.auto_reload_on_alarm = true;
    ESP_ERROR_CHECK(gptimer_set_alarm_action(g_foc_timer, &alarm));
    ESP_ERROR_CHECK(gptimer_start(g_foc_timer));

    ESP_LOGI(TAG, "FOC ISR up at %u Hz", (unsigned)FOC_ISR_HZ);
}

// Non-blocking nSLEEP fault-recovery state machine — mirrors M4's resetDriverBegin/Step
// pair (.ino:253–271). Pulses nSLEEP low for 50 µs to clear DRV8313 latched OCP/TSD,
// waits NSP_WAKE_US for the part to come out of sleep, re-syncs electrical_angle from the
// sensor (in case the rotor slipped during a stall), then re-enables the driver. Driven
// by reset_driver_step() at the top of motor_tick(); never blocks the task.
constexpr uint32_t NSP_LOW_US  = 50;
constexpr uint32_t NSP_WAKE_US = 2050;
enum class RecoverState : uint8_t { Idle, NSpLow, Settling };
static RecoverState s_recover_state = RecoverState::Idle;
static int64_t      s_recover_t0_us = 0;

static void halt_motor() {
    g_motor.disable();
    g_motor.PID_velocity.reset();
    g_motor.P_angle.reset();
    g_motor.shaft_velocity = 0.0f;
}

static void reset_driver_begin() {
    halt_motor();
    gpio_set_level((gpio_num_t)PIN_NSP, 0);
    s_recover_t0_us = esp_timer_get_time();
    s_recover_state = RecoverState::NSpLow;
}

static void reset_driver_step() {
    if (s_recover_state == RecoverState::Idle) return;
    int64_t elapsed = esp_timer_get_time() - s_recover_t0_us;
    if (s_recover_state == RecoverState::NSpLow && elapsed >= NSP_LOW_US) {
        gpio_set_level((gpio_num_t)PIN_NSP, 1);
        s_recover_state = RecoverState::Settling;
    }
    if (s_recover_state == RecoverState::Settling && elapsed >= NSP_WAKE_US) {
        // Re-sync from the sensor before SVPWM resumes — if the rotor slipped during the
        // jam, the cached electrical_angle would drive the wrong phase and produce no torque.
        g_motor.electrical_angle = g_motor.electricalAngle();
        g_motor.enable();
        s_recover_state = RecoverState::Idle;
    }
}

void motor_init_and_foc() {
    if (g_init_done.load()) return;

    gpio_config_t nsp_io = {};
    nsp_io.pin_bit_mask = (1ULL << PIN_NSP);
    nsp_io.mode         = GPIO_MODE_OUTPUT;
    nsp_io.pull_up_en   = GPIO_PULLUP_DISABLE;
    nsp_io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    nsp_io.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&nsp_io));
    gpio_set_level((gpio_num_t)PIN_NSP, 1);

    g_encoder.quadrature = Quadrature::ON;
    g_encoder.pullup     = Pullup::USE_INTERN;
    g_encoder.init();
    g_encoder.enableInterrupts(enc_a_isr, enc_b_isr);
    g_motor.linkSensor(&g_encoder);

    g_driver.voltage_power_supply = SUPPLY_VOLTAGE;
    g_driver.voltage_limit        = SUPPLY_VOLTAGE;
    g_driver.pwm_frequency        = PWM_FREQ_HZ;
    int driver_ok = g_driver.init(0);
    ESP_LOGI(TAG, "driver.init() = %d", driver_ok);
    g_motor.linkDriver(&g_driver);

    // Intentionally NOT setting phase_resistance — that flips arduino-foc 2.3.0's voltage-mode
    // behavior to match 2.4.0's: PID_velocity.limit becomes voltage_limit (~12V default) instead
    // of current_limit (0.5A) and move() outputs voltage.q = current_sp directly instead of
    // current_sp * R + bemf. Net effect: PID can drive up to ~12V instead of being capped at
    // ~3V, which is the user-visible "torque is much weaker than M4 / motor-only" complaint.
    // KV_rating still set so BEMF is computed for future reference; current_limit kept for any
    // future current-mode controller.
    // g_motor.phase_resistance  = PHASE_RESISTANCE;
    g_motor.KV_rating            = KV_RATING;
    g_motor.current_limit        = CURRENT_LIMIT;
    g_motor.voltage_sensor_align = VOLTAGE_SENSOR_ALIGN;
    g_motor.controller           = MotionControlType::angle;
    g_motor.torque_controller    = TorqueControlType::voltage;
    g_motor.foc_modulation       = FOCModulationType::SpaceVectorPWM;
    g_motor.motion_downsample    = MOTION_DOWNSAMPLE;

    g_motor.PID_velocity.P           = PID_VEL_P;
    g_motor.PID_velocity.I           = PID_VEL_I;
    g_motor.PID_velocity.D           = PID_VEL_D;
    g_motor.PID_velocity.output_ramp = PID_VEL_OUTPUT_RAMP;
    g_motor.LPF_velocity.Tf          = LPF_VEL_TF;
    g_motor.P_angle.P                = PID_ANGLE_P;

    g_motor.init();
    ESP_LOGI(TAG, "motor.init() done");

    // Always run fresh alignSensor each boot. Persisting the calibration was tried and
    // failed badly — the saved sensor_direction could become invalid between sessions
    // (producing a runaway where the motor thinks it's correcting toward target but
    // actually drives away from it, observed as shaft_angle growing past 80 rad with a
    // commanded target of 9). Motor-only build runs fresh alignment every boot and is
    // smooth, so we match that. If calibration is ever persisted again it must validate
    // the direction on boot (e.g., apply a small test voltage and check the sensor moves
    // in the expected direction) before trusting the saved value.
    persistence_clear_foc_calibration();
    int foc_ok = g_motor.initFOC();
    ESP_LOGI(TAG, "motor.initFOC() = %d  zero_elec=%.4f  dir=%d",
             foc_ok, (double)g_motor.zero_electric_angle, (int)g_motor.sensor_direction);
    if (!foc_ok) ESP_LOGE(TAG, "motor.initFOC() FAILED — alignment didn't converge");

    g_motor.velocity_limit = g_velocity_limit;

    float cmd = g_commanded.load();
    g_motor.sensor_offset -= cmd;
    g_target.store(cmd);
    g_target_vel.store(0.0f);
    g_motor.loopFOC();

    g_init_done.store(true);
    ESP_LOGI(TAG, "motor + FOC ready (anchored cmd=%.2f)", (double)cmd);

    // Start the 1 kHz commutation ISR as the last init step — idempotent, safe to call
    // from any init path (commissioning-complete event, post-reboot, short-click bench).
    motor_start_foc_timer();
}

bool motor_is_ready()             { return g_init_done.load(); }
void motor_set_commanded(float r) {
    if (r < POS_MIN) r = POS_MIN;
    if (r > POS_MAX) r = POS_MAX;
    g_commanded.store(r);
}
float motor_get_commanded()       { return g_commanded.load(); }
float motor_get_shaft_angle()     { return g_init_done.load() ? g_motor.shaft_angle : 0.0f; }

void motor_set_velocity_preset(uint8_t idx) {
    if (idx >= VELOCITY_PRESET_N) idx = DEFAULT_SPEED_IDX;
    g_velocity_limit = VELOCITY_PRESETS[idx];
    if (g_init_done.load()) g_motor.velocity_limit = g_velocity_limit;
    ESP_LOGI(TAG, "velocity preset = %.1f rad/s", (double)g_velocity_limit);
}

bool motor_is_enabled() { return g_init_done.load() && g_motor.enabled; }

// Trapezoidal velocity profile — matches Arduino M3b/M4 exactly. Without this the motor
// sprints straight at velocity_limit, which at 22 rad/s produces encoder edges every ~68 µs
// — faster than our ~200 µs FOC ISR can yield to the MT6701 GPIO ISR, so shaft_angle freezes
// and the stall detector fires at specific shaft angles.
static void ramp_target() {
    static int64_t last_us = 0;
    int64_t now_us = esp_timer_get_time();
    if (last_us == 0) { last_us = now_us; return; }
    float dt = (float)(now_us - last_us) * 1e-6f;
    last_us = now_us;
    if (dt <= 0 || dt > 0.1f) return;

    float cmd  = g_commanded.load();
    float tgt  = g_target.load();
    float tvel = g_target_vel.load();
    float err = cmd - tgt;
    float decel_dist = (tvel * tvel) / (2.0f * g_accel);
    float desired_vel = (fabsf(err) <= decel_dist) ? 0.0f
                                                   : (err > 0 ? 1.0f : -1.0f) * g_velocity_limit;

    float vel_step = g_accel * dt;
    if      (tvel < desired_vel - vel_step) tvel += vel_step;
    else if (tvel > desired_vel + vel_step) tvel -= vel_step;
    else                                    tvel  = desired_vel;

    tgt += tvel * dt;

    if ((err > 0 && tgt >= cmd) || (err < 0 && tgt <= cmd)) {
        tgt  = cmd;
        tvel = 0.0f;
    }
    g_target.store(tgt);
    g_target_vel.store(tvel);
}

static int64_t s_stall_start_us = 0;
static int64_t s_enable_time_us = 0;
static int64_t s_last_active_us = 0;
static std::atomic<bool> s_stall_synced{false};  // set on stall; app_driver consumes to push corrected position to Matter

bool motor_consume_stall_event() { return s_stall_synced.exchange(false); }

void motor_tick() {
    if (!g_init_done.load()) {
        leds_update(0.0f);
        return;
    }

    // Advance the non-blocking nSLEEP recovery first so its timer keeps ticking even if
    // nothing else in this iteration touches the motor.
    reset_driver_step();

    // Per-edge re-anchor (matches M4 .ino:780–788). Whenever the motor settles into the
    // disabled state — whether from idle timeout or a stall halt — snap commanded/target
    // to actual shaft position so subsequent user input is anchored to physical reality.
    static bool s_was_enabled = false;
    if (g_motor.enabled) {
        s_was_enabled = true;
    } else if (s_was_enabled && s_recover_state == RecoverState::Idle) {
        float a = g_motor.shaft_angle;
        g_commanded.store(a);
        g_target.store(a);
        g_target_vel.store(0.0f);
        s_was_enabled = false;
    }

    ramp_target();

    float cmd  = g_commanded.load();
    float tgt  = g_target.load();
    float tvel = g_target_vel.load();
    bool ramping     = fabsf(cmd - tgt) > 1e-4f || fabsf(tvel) > 1e-4f;
    // No `g_motor.enabled &&` qualifier — without a trapezoidal ramp, this is the only signal
    // that wakes the motor when Matter (or a knob in the idle-anchored case) sets a fresh target.
    bool pos_err_big = fabsf(tgt - g_motor.shaft_angle) > IDLE_POS_THRESH;
    bool want_motion = ramping || pos_err_big;

    int64_t now_us = esp_timer_get_time();

    if (want_motion) {
        s_last_active_us = now_us;
        if (!g_motor.enabled && s_recover_state == RecoverState::Idle) {
            // Re-enable via the non-blocking nSLEEP pulse — clears any latched DRV8313 OCP/TSD
            // before SVPWM resumes (M4 pattern: every disable→enable transition gets a fresh pulse).
            reset_driver_begin();
            s_enable_time_us = now_us;
            s_stall_start_us = 0;
        }
        bool in_warmup = (now_us - s_enable_time_us) < ((int64_t)STALL_WARMUP_MS * 1000);
        // Only consider stall when we're actively commanding motion (target_vel != 0). During natural
        // end-of-move deceleration, target_vel ramps to 0 and shaft velocity briefly dips — not a stall.
        bool actively_seeking = fabsf(tvel) > STALL_VEL_THRESHOLD;
        if (in_warmup) {
            s_stall_start_us = 0;  // don't carry stall accumulation across the warmup boundary
        } else if (actively_seeking && fabsf(g_motor.shaft_velocity) < STALL_VEL_THRESHOLD && pos_err_big) {
            if (s_stall_start_us == 0) {
                s_stall_start_us = now_us;
            } else if (now_us - s_stall_start_us > ((int64_t)STALL_TIMEOUT_MS * 1000)) {
                ESP_LOGW(TAG, "stall at ang=%.2f seeking cmd=%.2f — halt",
                         (double)g_motor.shaft_angle, (double)cmd);
                halt_motor();   // disable + flush PID; commanded snaps via per-edge anchor next tick
                s_stall_start_us = 0;
                s_stall_synced.store(true);   // app_driver will push the corrected position to Matter
            }
        } else {
            s_stall_start_us = 0;
        }

        // Diagnostic: every 250 ms while we're commanding motion, dump the stall-decision inputs
        // so a "tried to force a stall, didn't fire" failure is debuggable from serial.
        static int64_t s_diag_last_us = 0;
        if (now_us - s_diag_last_us > 250000) {
            s_diag_last_us = now_us;
            ESP_LOGI(TAG, "diag: tgt=%.2f ang=%.2f tvel=%.2f shvel=%.2f pos_err=%.2f warmup=%d stall_acc=%lldms",
                     (double)tgt, (double)g_motor.shaft_angle,
                     (double)tvel, (double)g_motor.shaft_velocity,
                     (double)(tgt - g_motor.shaft_angle), (int)in_warmup,
                     s_stall_start_us ? (long long)((now_us - s_stall_start_us) / 1000) : 0LL);
        }
    } else {
        s_stall_start_us = 0;
        if (g_motor.enabled && now_us - s_last_active_us > ((int64_t)IDLE_DISABLE_MS * 1000)) {
            g_motor.disable();
        }
    }

    // loopFOC runs in the GPTimer ISR (see foc_isr_cb). Task only updates voltage.q via move().
    g_motor.move(g_target.load());
    leds_update(g_motor.shaft_angle);
}
