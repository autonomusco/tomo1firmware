#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "i2c_bus.h"
#include "max17048.h"
#include "mpu6050.h"
#include "button.h"
#include "ble.h"

static const char *TAG = "APP";
static esp_timer_handle_t telemetry_timer;

static void telemetry_cb(void *arg) {
    float soc = max17048_get_soc();
    mpu6050_reading_t m = mpu6050_read();
    ble_update_battery((uint8_t)(soc + 0.5f));
    ble_update_motion(m.ax, m.ay, m.az);
    ESP_LOGI(TAG, "SOC: %.1f%% | accel: [%.2f, %.2f, %.2f]", soc, m.ax, m.ay, m.az);
}

static void alert_cb(button_event_t ev) {
    if (ev == BUTTON_EVENT_PRESS) {
        ESP_LOGW(TAG, "ALERT button pressed");
        ble_send_alert();
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(i2c_bus_init(GPIO_NUM_8, GPIO_NUM_9, 400000)); // SDA=8, SCL=9

    max17048_init();
    mpu6050_init();

    button_init(GPIO_NUM_0, alert_cb); // pull-up + ISR, active-low

    ble_init();        // sets up GAP/GATT, starts adv
    ble_set_device_name("TomoPendant");
    ble_start_advertising();

    const esp_timer_create_args_t tcfg = {
        .callback = telemetry_cb, .name = "telemetry"
    };
    ESP_ERROR_CHECK(esp_timer_create(&tcfg, &telemetry_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(telemetry_timer, 5 * 1000 * 1000)); // 5s

    // Optionally enable light sleep between events:
    // esp_sleep_enable_timer_wakeup(1000 * 1000);
    // esp_light_sleep_start();
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

    // Init emergency button
    if (!emergency_button_init(BUTTON_GPIO)) {
        ESP_LOGE(TAG, "Emergency button init failed!");
    }

    // Init BLE pairing
    if (!ble_pairing_init()) {
        ESP_LOGE(TAG, "BLE pairing init failed!");
    }

    ESP_LOGI(TAG, "System running.");

    while (1) {
        float v = 0, soc = 0;
        if (power_mgmt_get_voltage(&v) && power_mgmt_get_soc(&soc)) {
            ESP_LOGI(TAG, "Battery: %.3f V, %.1f%%", v, soc);
            ble_pairing_update_battery(soc);
        }
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
