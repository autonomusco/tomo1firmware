#include "cloud_api.h"
#include "esp_log.h"

static const char *TAG = "CLOUD";

void cloud_api_init(void) {
    ESP_LOGI(TAG, "Cloud API initialized (stub)");
}

bool cloud_api_send_telemetry(float voltage, float soc,
                              float ax, float ay, float az) {
    ESP_LOGI(TAG, "Telemetry -> Cloud (stub): V=%.2f SOC=%.1f [%.2f, %.2f, %.2f]",
             voltage, soc, ax, ay, az);
    return true; // stub always succeeds
}

bool cloud_api_send_alert(uint8_t code) {
    ESP_LOGW(TAG, "ALERT -> Cloud (stub): code=0x%02X", code);
    return true; // stub always succeeds
}
