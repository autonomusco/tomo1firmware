#include "ota_update.h"
#include "esp_log.h"

static const char *TAG = "OTA";

/**
 * @brief Initialize OTA update subsystem.
 * Currently a stub, reserved for Wi-Fi and OTA setup.
 */
void ota_update_init(void) {
    ESP_LOGI(TAG, "OTA subsystem initialized (stub)");
}

/**
 * @brief Start OTA update.
 * Currently a stub, will later fetch and apply firmware.
 */
void ota_update_start(void) {
    ESP_LOGI(TAG, "OTA update start (stub)");
}
