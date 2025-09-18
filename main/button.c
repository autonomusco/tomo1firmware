#include "button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static button_cb_t s_cb;
static int64_t down_ts = 0;

static void IRAM_ATTR isr(void *arg) {
    int level = gpio_get_level((gpio_num_t)(intptr_t)arg);
    int64_t now = esp_timer_get_time();
    if (level == 0) { // pressed
        down_ts = now;
    } else { // released
        int64_t dur_ms = (now - down_ts)/1000;
        if (s_cb) s_cb(dur_ms > 1500 ? BUTTON_EVENT_LONG : BUTTON_EVENT_PRESS);
    }
}

void button_init(gpio_num_t gpio, button_cb_t cb) {
    s_cb = cb;
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1, .pull_down_en = 0, .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&cfg);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio, isr, (void*)(intptr_t)gpio);
}
