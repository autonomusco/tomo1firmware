#include "health_sensors.h"
#include "esp_log.h"

static const char *TAG = "HEALTH";

/**
 * Initialize health sensors (stub for now).
 */
void health_sensors_init(void) {
    ESP_LOGI(TAG, "Health sensors initialized (stub).");
    // TODO: add actual sensor initialization later
}

/**
 * Return dummy health data until real sensors are integrated.
 */
health_reading_t health_sensors_read(void) {
    health_reading_t reading = {
        .spo2 = 98.5f,  // Example value
        .hr   = 72.0f   // Example value
    };

    ESP_LOGI(TAG, "Stub health read -> SpO2=%.1f%% HR=%.1f bpm",
             reading.spo2, reading.hr);

    return reading;
}
