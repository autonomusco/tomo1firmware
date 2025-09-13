#include "cloud_api.h"
#include "esp_log.h"

static const char *TAG = "CLOUD_API";

void cloud_api_init(void) {
    ESP_LOGI(TAG, "cloud_api_init called");
}

void cloud_api_send_event(const char *event) {
    ESP_LOGI(TAG, "cloud_api_send_event called with %s", event);
}
