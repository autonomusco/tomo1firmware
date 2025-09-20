#include "diagnostics.h"
#include "esp_log.h"

static const char *TAG = "DIAG";

void diagnostics_event(const char *msg, const char *context) {
    if (msg) {
        if (context) {
            ESP_LOGI(TAG, "Event: %s | Context: %s", msg, context);
        } else {
            ESP_LOGI(TAG, "Event: %s", msg);
        }
    } else {
        ESP_LOGW(TAG, "diagnostics_event called with NULL msg");
    }
}

void diagnostics_self_test(void) {
    // CI-friendly stub to validate both APIs
    diagnostics_event("Self-test executed", "CI validation");
    diagnostics_log_event("Self-test alias executed");
}
