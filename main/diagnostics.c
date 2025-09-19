#include "diagnostics.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DIAGNOSTICS";

static inline uint64_t ms_since_boot(void) {
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

void diagnostics_init(void) {
    ESP_LOGI(TAG, "Diagnostics init @ %llu ms", (unsigned long long)ms_since_boot());
}

void diagnostics_event(const char *event_name, const char *detail) {
    if (!event_name) event_name = "unknown";
    if (!detail) detail = "";
    ESP_LOGI(TAG, "[%llu ms] EVENT: %s | %s",
             (unsigned long long)ms_since_boot(), event_name, detail);
}

void diagnostics_error(const char *where, esp_err_t err, const char *detail) {
    if (!where) where = "unknown";
    if (!detail) detail = "";
    ESP_LOGE(TAG, "[%llu ms] ERROR @ %s: %s (0x%x)",
             (unsigned long long)ms_since_boot(), where, detail, (unsigned)err);
}
