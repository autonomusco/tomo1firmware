#include "diagnostics.h"
#include "esp_log.h"

static const char *TAG = "DIAG";

/**
 * Basic diagnostics logging function.
 * In the future this can be extended to send logs to cloud or store locally.
 */
void diagnostics_log_event(const char *msg) {
    if (msg) {
        ESP_LOGI(TAG, "Event: %s", msg);
    } else {
        ESP_LOGW(TAG, "diagnostics_log_event called with NULL msg");
    }
}
