#include "emergency_button.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_attr.h"

#define TAG "EMERGENCY_BUTTON"

// ISR handler
static void IRAM_ATTR emergency_button_isr_handler(void *arg) {
    int gpio_num = (int)(intptr_t)arg;
    // Use ISR-safe logging
    ESP_EARLY_LOGI(TAG, "Emergency button pressed! (GPIO %d)", gpio_num);
}

bool emergency_button_init(int gpio_num) {
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << gpio_num,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,  // Trigger on button press (falling edge)
    };

    if (gpio_config(&io_conf) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d", gpio_num);
        return false;
    }

    // Install ISR service if not already installed
    if (gpio_install_isr_service(0) != ESP_OK) {
        ESP_LOGW(TAG, "ISR service already installed");
    }

    // Add ISR handler for this button GPIO
    if (gpio_isr_handler_add(gpio_num, emergency_button_isr_handler,
                             (void *)(intptr_t)gpio_num) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d", gpio_num);
        return false;
    }

    ESP_LOGI(TAG, "Emergency button initialized on GPIO %d", gpio_num);
    return true;
}