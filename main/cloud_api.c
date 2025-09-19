#include "cloud_api.h"
#include "esp_log.h"

static const char *TAG = "CLOUD_API";

void cloud_api_init(void) {
    ESP_LOGI(TAG, "Cloud API initialized (stub)");
}

void cloud_api_send_telemetry(float voltage, float soc,
                              float ax, float ay, float az) {
    ESP_LOGI(TAG, "Telemetry -> V=%.2fV SOC=%.1f%% accel[g]=[%.2f, %.2f, %.2f]",
             voltage, soc, ax, ay, az);
    // TODO: Add real cloud comms (HTTP/MQTT) in Stage 9+
}

void cloud_api_send_alert(uint8_t code) {
    ESP_LOGW(TAG, "Alert -> code=0x%02X", code);
    // TODO: Add real cloud comms in Stage 9+
}
