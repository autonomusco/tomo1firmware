#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// BLE
#include "ble.h"
#include "ble_pairing.h"
#include "ble_advanced.h"

// Buttons
#include "button.h"
#include "emergency_button.h"

// Power & sensors
#include "power_mgmt.h"
#include "health_sensors.h"

// Other subsystems
#include "diagnostics.h"
#include "ota_update.h"
#include "cloud_api.h"

static const char *TAG = "APP_MAIN";

// ✅ Fixed callback signature
static void emergency_cb(emergency_event_t ev) {
    if (ev == EMERGENCY_EVENT_PRESS) {
        ESP_LOGW(TAG, "Emergency button pressed");
        ble_send_alert_code(0x03);
        cloud_api_send_alert(0x03);
    } else if (ev == EMERGENCY_EVENT_LONG) {
        ESP_LOGW(TAG, "Emergency button long press -> OTA");
        ble_send_alert_code(0x13);
        ota_update_start();
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=== TOMO Firmware Starting ===");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Power management
    ESP_LOGI(TAG, "Init power management");
    power_mgmt_init();

    float voltage = 0.0f, soc = 0.0f;
    if (power_mgmt_get_voltage(&voltage)) {
        ESP_LOGI(TAG, "Battery voltage: %.2f V", voltage);
    }
    if (power_mgmt_get_soc(&soc)) {
        ESP_LOGI(TAG, "Battery SOC: %.2f %%", soc);
    }

    // Emergency button
    ESP_LOGI(TAG, "Init emergency button");
    emergency_button_init(GPIO_NUM_0, emergency_cb);

    // BLE
    ESP_LOGI(TAG, "Init BLE pairing and services");
    ble_pairing_init();
    ble_advanced_init();   // Stage 9: add config + OTA trigger services

    // OTA subsystem
    ESP_LOGI(TAG, "Init OTA update stubs");
    ota_update_init();

    // Health sensors
    ESP_LOGI(TAG, "Init health sensors (stub)");
    health_sensors_init();
    health_reading_t hr = health_sensors_read();
    ESP_LOGI(TAG, "Initial health reading: SpO2=%.1f%%, HR=%.1f bpm", hr.spo2, hr.hr);

    // Cloud API
    ESP_LOGI(TAG, "Init cloud API stubs");
    cloud_api_init();
    cloud_api_send_telemetry(voltage, soc, 0.0f, 0.0f, 0.0f);

    // ✅ Fixed diagnostics call
    ESP_LOGI(TAG, "Init diagnostics logging");
    diagnostics_event("System init complete", NULL);

    ESP_LOGI(TAG, "=== TOMO Firmware Ready ===");
}
