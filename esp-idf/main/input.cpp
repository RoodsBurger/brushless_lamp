#include "input.h"
#include "pins.h"
#include "config.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"
#include "esp_timer.h"

#include <stdint.h>

static const char *TAG = "input";

static InputCallbacks    s_cbs{};
static pcnt_unit_handle_t s_pcnt = nullptr;
static int                s_pcnt_prev = 0;

// Button state machine
static bool     s_btn_raw = false;
static bool     s_btn_debounced = false;
static int64_t  s_btn_last_edge_us = 0;
static int64_t  s_press_start_us = 0;
static int64_t  s_first_release_us = 0;
static bool     s_warn_fired = false;
static bool     s_long_reset_done = false;
enum class ClickStage { Idle, WaitDouble };
static ClickStage s_click_stage = ClickStage::Idle;

static void pcnt_setup() {
    pcnt_unit_config_t unit_cfg = {};
    unit_cfg.low_limit  = -32768;
    unit_cfg.high_limit =  32767;
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_cfg, &s_pcnt));

    pcnt_glitch_filter_config_t filter_cfg = { .max_glitch_ns = 10000 };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(s_pcnt, &filter_cfg));

    pcnt_chan_config_t chan_a_cfg = {};
    chan_a_cfg.edge_gpio_num  = PIN_ROT_A;
    chan_a_cfg.level_gpio_num = PIN_ROT_B;
    pcnt_channel_handle_t chan_a;
    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt, &chan_a_cfg, &chan_a));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    pcnt_chan_config_t chan_b_cfg = {};
    chan_b_cfg.edge_gpio_num  = PIN_ROT_B;
    chan_b_cfg.level_gpio_num = PIN_ROT_A;
    pcnt_channel_handle_t chan_b;
    ESP_ERROR_CHECK(pcnt_new_channel(s_pcnt, &chan_b_cfg, &chan_b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_ERROR_CHECK(pcnt_unit_enable(s_pcnt));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(s_pcnt));
    ESP_ERROR_CHECK(pcnt_unit_start(s_pcnt));
    s_pcnt_prev = 0;
}

void input_init(const InputCallbacks &cbs) {
    s_cbs = cbs;

    gpio_config_t btn_io = {};
    btn_io.pin_bit_mask = (1ULL << PIN_BTN);
    btn_io.mode         = GPIO_MODE_INPUT;
    btn_io.pull_up_en   = GPIO_PULLUP_ENABLE;
    btn_io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    btn_io.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&btn_io));

    gpio_config_t rot_io = {};
    rot_io.pin_bit_mask = (1ULL << PIN_ROT_A) | (1ULL << PIN_ROT_B);
    rot_io.mode         = GPIO_MODE_INPUT;
    rot_io.pull_up_en   = GPIO_PULLUP_ENABLE;
    rot_io.pull_down_en = GPIO_PULLDOWN_DISABLE;
    rot_io.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&rot_io));

    pcnt_setup();
    ESP_LOGI(TAG, "input init: btn=GPIO%u, rot=GPIO%u/%u (PCNT)",
             (unsigned)PIN_BTN, (unsigned)PIN_ROT_A, (unsigned)PIN_ROT_B);
}

static void poll_knob() {
    int now = 0;
    pcnt_unit_get_count(s_pcnt, &now);
    int delta = now - s_pcnt_prev;
    if (delta == 0) return;
    int detents = delta / 4;        // 4 PCNT counts per mechanical detent
    if (detents == 0) return;
    s_pcnt_prev += detents * 4;
    if (s_cbs.on_knob) s_cbs.on_knob(detents);
}

static inline int64_t ms_since(int64_t us) {
    return (esp_timer_get_time() - us) / 1000;
}

static void poll_button() {
    int64_t now_us = esp_timer_get_time();
    bool raw = (gpio_get_level((gpio_num_t)PIN_BTN) == 0);
    if (raw != s_btn_raw) { s_btn_raw = raw; s_btn_last_edge_us = now_us; }

    if (((now_us - s_btn_last_edge_us) / 1000) > BTN_DEBOUNCE_MS && raw != s_btn_debounced) {
        s_btn_debounced = raw;
        if (s_btn_debounced) {
            s_press_start_us  = now_us;
            s_warn_fired      = false;
            s_long_reset_done = false;
        } else {
            int64_t held_ms = (now_us - s_press_start_us) / 1000;
            if (s_long_reset_done) {
                // already fired
            } else if (held_ms < BTN_LONG_WARN_MS) {
                if (s_click_stage == ClickStage::WaitDouble &&
                    ((now_us - s_first_release_us) / 1000) < BTN_DBL_GAP_MS) {
                    if (s_cbs.on_double_click) s_cbs.on_double_click();
                    s_click_stage = ClickStage::Idle;
                } else {
                    s_first_release_us = now_us;
                    s_click_stage = ClickStage::WaitDouble;
                }
            } else {
                s_click_stage = ClickStage::Idle;
            }
        }
    }

    if (s_click_stage == ClickStage::WaitDouble && !s_btn_debounced &&
        ((now_us - s_first_release_us) / 1000) > BTN_DBL_GAP_MS) {
        if (s_cbs.on_short_click) s_cbs.on_short_click();
        s_click_stage = ClickStage::Idle;
    }

    if (s_btn_debounced && !s_long_reset_done) {
        int64_t held_ms = (now_us - s_press_start_us) / 1000;
        if (held_ms >= BTN_LONG_RESET_MS) {
            s_long_reset_done = true;
            if (s_cbs.on_long_press_reset) s_cbs.on_long_press_reset();
        } else if (held_ms >= BTN_LONG_WARN_MS && !s_warn_fired) {
            s_warn_fired = true;
            if (s_cbs.on_long_press_warn) s_cbs.on_long_press_warn();
        }
    }
}

void input_poll() {
    poll_knob();
    poll_button();
}
