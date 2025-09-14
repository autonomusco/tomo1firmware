#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "fall_detection.h"
#include "power_mgmt.h"

#define TAG "APP_MAIN"

// I2C pins — adjust to your board
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

static void i2c_bus_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,  // 100kHz
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "I2C bus initialized (SDA=%d, SCL=%d)", I2C_SDA_PIN, I2C_SCL_PIN);
}

void app_main(void) {
    ESP_LOGI(TAG, "System booting…");

    // Init I2C
    i2c_bus_init();

    // Init fall detection
    if (!fall_detection_init()) {
        ESP_LOGE(TAG, "Fall detection init failed!");
    }

    // Init power management
    if (!power_mgmt_init()) {
        ESP_LOGE(TAG, "Power management init failed!");
    }

    ESP_LOGI(TAG, "System running.");

    // Periodically log battery
    while (1) {
        float v = 0, soc = 0;
        if (power_mgmt_get_voltage(&v) && power_mgmt_get_soc(&soc)) {
            ESP_LOGI(TAG, "Battery: %.3f V, %.1f%%", v, soc);
        } else {
            ESP_LOGW(TAG, "Battery read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(10000)); // every 10s
    }
}
