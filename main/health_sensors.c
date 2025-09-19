#include "health_sensors.h"
#include "esp_log.h"

static const char *TAG = "HEALTH_SENSORS";

esp_err_t health_sensors_init(void) {
    ESP_LOGI(TAG, "Health sensors initialized (stub)");
    return ESP_OK;
}

health_reading_t health_sensors_read(void) {
    health_reading_t r = {
        .spo2 = 98.0f,   // stub values
        .hr   = 72.0f
    };
    ESP_LOGI(TAG, "Health reading (stub): SpO2=%.1f%% HR=%.1f bpm",
             r.spo2, r.hr);
    return r;
}
