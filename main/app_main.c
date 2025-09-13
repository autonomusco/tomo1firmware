#include "ble_pairing.h"
#include "cloud_api.h"
#include "diagnostics.h"
#include "emergency_button.h"
#include "fall_detection.h"
#include "health_sensors.h"
#include "i2c_bus.h"
#include "ota_update.h"
#include "power_mgmt.h"
#include "security.h"
#include "voice_trigger.h"

#include "esp_log.h"

static const char *TAG = "TOMO_MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "===== Starting TOMO firmware sanity test =====");

    ESP_LOGI(TAG, "Initializing I2C bus...");
    i2c_bus_init();

    ESP_LOGI(TAG, "Initializing power management...");
    power_mgmt_init();

    ESP_LOGI(TAG, "Initializing security...");
    security_init();

    ESP_LOGI(TAG, "Initializing BLE pairing...");
    ble_pairing_init();

    ESP_LOGI(TAG, "Initializing fall detection...");
    fall_detection_init();

    ESP_LOGI(TAG, "Initializing health sensors...");
    health_sensors_init();

    ESP_LOGI(TAG, "Initializing emergency button...");
    emergency_button_init();

    ESP_LOGI(TAG, "Initializing voice trigger...");
    voice_trigger_init();

    ESP_LOGI(TAG, "Running diagnostics...");
    diagnostics_run();

    ESP_LOGI(TAG, "Starting OTA update...");
    ota_update_start();

    ESP_LOGI(TAG, "Initializing cloud API...");
    cloud_api_init();

    ESP_LOGI(TAG, "===== System initialization complete =====");
}
