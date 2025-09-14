#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Fall detection configuration thresholds
 */
#ifndef FD_IMPACT_G
#define FD_IMPACT_G        2.5f   // g threshold for impact spike
#endif

#ifndef FD_QUIET_G
#define FD_QUIET_G         0.5f   // g threshold for post-impact stillness
#endif

#ifndef FD_SETTLE_MS
#define FD_SETTLE_MS       800    // ms window to confirm stillness after impact
#endif

#ifndef FD_SAMPLE_PERIOD_MS
#define FD_SAMPLE_PERIOD_MS 50    // polling interval in ms
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t ax, ay, az;     // raw accel data
    int16_t gx, gy, gz;     // raw gyro data
    float   a_mag_g;        // accel magnitude in g
    uint32_t ts_ms;         // timestamp (ms)
} fall_event_t;

/**
 * Initialize the fall detection system and start background task.
 * Returns true on success, false on failure.
 */
bool fall_detection_init(void);

/**
 * Get the last detected fall event (if any).
 */
bool fall_detection_get_last(fall_event_t *out);

/**
 * Update thresholds at runtime.
 */
void fall_detection_set_thresholds(float impact_g, float quiet_g, uint32_t settle_ms);

#ifdef __cplusplus
}
#endif

#endif // FALL_DETECTION_H
