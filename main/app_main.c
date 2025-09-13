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
    ESP_LOGI(TAG, "Starting TOMO firmware...");

    i2c_bus_init();
    power_mgmt_init();
    security_init();

    ble_pairing_init();
    fall_detection_init();
    health_sensors_init();
    emergency_button_init();
    voice_trigger_init();
    diagnostics_run();

    ota_update_start();
    cloud_api_init();

    ESP_LOGI(TAG, "System initialization complete.");
}
