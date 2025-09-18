#include "button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_attr.h"

static const char *TAG = "BUTTON";
static button_cb_t cb = NULL;
static gpio_num_t btn_gpio;

static void IRAM_ATTR isr(void *arg) {
    gpio_num_t gpio = (gpio_num_t)(intptr_t)arg;
    if (gpio == btn_gpio && cb) {
        cb(BUTTON_EVENT_PRESS);
    }
}

void button_init(gpio_num_t gpio, button_cb_t callback) {
    btn_gpio = gpio;
    cb = callback;

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << gpio),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(gpio, isr, (void*)(intptr_t)gpio);

    ESP_LOGI(TAG, "Button initialized on GPIO %d", gpio);
}
