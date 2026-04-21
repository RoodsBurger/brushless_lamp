#include "motor.h"
#include "leds.h"
#include "pins.h"
#include "config.h"

#include "esp_simplefoc.h"
#include "sensors/Encoder.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

#include <atomic>
#include <math.h>

static const char *TAG = "motor";

static BLDCMotor       g_motor   = BLDCMotor(POLE_PAIRS);
static BLDCDriver3PWM  g_driver  = BLDCDriver3PWM(PIN_PWM_A, PIN_PWM_B, PIN_PWM_C);
static Encoder         g_encoder = Encoder(PIN_ENC_A, PIN_ENC_B, ENCODER_CPR);

static std::atomic<bool> g_init_done{false};

static std::atomic<float> g_commanded{0.0f};
static std::atomic<float> g_target{0.0f};       // task writes via ramp_target(); FOC ISR reads for motor.move()
static float              g_target_vel = 0.0f;
static float              g_accel = ACCEL_DEFAULT;
static float              g_velocity_limit = VELOCITY_PRESETS[DEFAULT_SPEED_IDX];

static gptimer_handle_t g_foc_timer = nullptr;

static void IRAM_ATTR enc_a_isr() { g_encoder.handleA(); }
static void IRAM_ATTR enc_b_isr() { g_encoder.handleB(); }

// loopFOC only in ISR — matches Arduino M4. Putting motor.move() in here too pushed the
// ISR to ~300 µs on C6 soft-float, which blocked MT6701 encoder GPIO interrupts (~61 µs
// period at full speed) and dropped encoder counts.
static bool IRAM_ATTR foc_isr_cb(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *) {
    g_motor.loopFOC();
    return false;
}

static void start_foc_timer() {
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

    ESP_LOGI(TAG, "FOC ISR up at %u Hz (period %u µs)",
             (unsigned)FOC_ISR_HZ, (unsigned)(1000000 / FOC_ISR_HZ));
}

// Stall-only path: pulse nSLEEP to clear any latched DRV8313 OCP/TSD, zero PID, re-enable.
static void fault_recover() {
    g_motor.disable();
    g_motor.PID_velocity.reset();
    g_motor.P_angle.reset();
    g_motor.shaft_velocity = 0.0f;
    gpio_set_level((gpio_num_t)PIN_NSP, 0);
    esp_rom_delay_us(50);
    gpio_set_level((gpio_num_t)PIN_NSP, 1);
    esp_rom_delay_us(2000);
    g_motor.enable();
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

    g_motor.phase_resistance     = PHASE_RESISTANCE;
    g_motor.KV_rating            = KV_RATING;
    g_motor.current_limit        = CURRENT_LIMIT;
    // Do not set motor.voltage_limit here — leave it at the SimpleFOC default (12 V) so peak phase current matches the Arduino build (2.4 A vs the 1.2 A we had with a 6 V cap).
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
    g_motor.LPF_angle.Tf             = LPF_ANGLE_TF;
    g_motor.P_angle.P                = PID_ANGLE_P;

    g_motor.init();
    ESP_LOGI(TAG, "motor.init() done");
    int foc_ok = g_motor.initFOC();
    ESP_LOGI(TAG, "motor.initFOC() = %d  zero_elec=%.4f  dir=%d",
             foc_ok, (double)g_motor.zero_electric_angle, (int)g_motor.sensor_direction);

    g_motor.velocity_limit = g_velocity_limit;

    float cmd = g_commanded.load();
    g_motor.sensor_offset -= cmd;
    g_target.store(cmd);
    g_target_vel = 0.0f;
    g_motor.enable();
    g_motor.loopFOC();

    start_foc_timer();
    g_init_done.store(true);
    ESP_LOGI(TAG, "motor + FOC ready (anchored cmd=%.2f)", (double)cmd);
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

// Trapezoidal velocity profile w/ direction-reversal-decel.
static void ramp_target() {
    static int64_t last_us = 0;
    int64_t now_us = esp_timer_get_time();
    if (last_us == 0) { last_us = now_us; return; }
    float dt = (float)(now_us - last_us) * 1e-6f;
    last_us = now_us;
    if (dt <= 0 || dt > 0.1f) return;

    float cmd = g_commanded.load();
    float t   = g_target.load();
    float err = cmd - t;
    bool wrong_direction = (err > 0 && g_target_vel < 0) || (err < 0 && g_target_vel > 0);
    float effective_accel = wrong_direction ? (g_accel * 4.0f) : g_accel;

    float decel_dist = (g_target_vel * g_target_vel) / (2.0f * effective_accel);
    float desired_vel = (fabsf(err) <= decel_dist) ? 0.0f
                                                   : (err > 0 ? 1.0f : -1.0f) * g_velocity_limit;

    float vel_step = effective_accel * dt;
    if      (g_target_vel < desired_vel - vel_step) g_target_vel += vel_step;
    else if (g_target_vel > desired_vel + vel_step) g_target_vel -= vel_step;
    else                                            g_target_vel  = desired_vel;

    t += g_target_vel * dt;

    if ((err > 0 && t >= cmd) || (err < 0 && t <= cmd)) {
        t = cmd;
        g_target_vel = 0.0f;
    }
    g_target.store(t);
}

static int64_t s_stall_start_us = 0;
static int64_t s_motion_start_us = 0;

void motor_tick() {
    if (!g_init_done.load()) {
        leds_update(0.0f);
        return;
    }

    ramp_target();

    float cmd = g_commanded.load();
    float t   = g_target.load();
    bool ramping     = fabsf(cmd - t) > 1e-4f || fabsf(g_target_vel) > 1e-4f;
    bool pos_err_big = fabsf(t - g_motor.shaft_angle) > IDLE_POS_THRESH;
    bool want_motion = ramping || pos_err_big;

    int64_t now_us = esp_timer_get_time();

    if (want_motion) {
        if (s_motion_start_us == 0) {
            s_motion_start_us = now_us;
            s_stall_start_us  = 0;
        }
        bool in_warmup = (now_us - s_motion_start_us) < ((int64_t)STALL_WARMUP_MS * 1000);
        if (!in_warmup && fabsf(g_motor.shaft_velocity) < STALL_VEL_THRESHOLD && pos_err_big) {
            if (s_stall_start_us == 0) {
                s_stall_start_us = now_us;
            } else if (now_us - s_stall_start_us > ((int64_t)STALL_TIMEOUT_MS * 1000)) {
                ESP_LOGW(TAG, "stall at ang=%.2f seeking cmd=%.2f — fault recovery",
                         (double)g_motor.shaft_angle, (double)cmd);
                g_commanded.store(g_motor.shaft_angle);
                g_target.store(g_motor.shaft_angle);
                g_target_vel = 0.0f;
                s_stall_start_us = 0;
                s_motion_start_us = now_us;
                fault_recover();
            }
        } else {
            s_stall_start_us = 0;
        }
    } else {
        s_stall_start_us  = 0;
        s_motion_start_us = 0;
    }

    g_motor.move(g_target.load());
    leds_update(g_motor.shaft_angle);
}
