#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#include "fall_detection.h"

#define TAG "APP_MAIN"

// I2C pin configuration — update if your hardware uses different pins
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

    // Step 1: Init I2C
    i2c_bus_init();

    // Step 2: Init fall detection
    if (!fall_detection_init()) {
        ESP_LOGE(TAG, "Fall detection init failed! Check MPU6050 wiring.");
        return;
    }

    ESP_LOGI(TAG, "System running. Fall detection task active.");

    // Step 3: Idle loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
