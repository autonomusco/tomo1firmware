#include "emergency_button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_attr.h"

static const char *TAG = "EMERGENCY_BTN";
static emergency_cb_t s_cb = NULL;
static gpio_num_t s_gpio;
static int64_t s_press_start = 0;

static void IRAM_ATTR isr(void *arg) {
    int level = gpio_get_level(s_gpio);
    int64_t now = esp_timer_get_time();

    if (level == 0) { // pressed
        s_press_start = now;
    } else { // released
        if (s_cb) {
            int64_t dur_ms = (now - s_press_start) / 1000;
            if (dur_ms > 1500) {
                s_cb(EMERGENCY_EVENT_LONG);
            } else {
                s_cb(EMERGENCY_EVENT_PRESS);
            }
        }
    }
}

void emergency_button_init(gpio_num_t gpio, emergency_cb_t cb) {
    s_gpio = gpio;
    s_cb = cb;

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpio, isr, NULL));

    ESP_LOGI(TAG, "Emergency button initialized on GPIO %d", gpio);
}
