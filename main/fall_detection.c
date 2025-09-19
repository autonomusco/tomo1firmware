#include "fall_detection.h"
#include "esp_log.h"

static const char *TAG = "FALL_DETECT";

void fall_detection_init(void) {
    ESP_LOGI(TAG, "Fall detection initialized (stub)");
}

bool fall_detection_check(const mpu6050_reading_t *m) {
    (void)m;  // suppress unused variable warning
    // TODO: Implement threshold-based fall detection
    return false;   // always safe until implemented
}
