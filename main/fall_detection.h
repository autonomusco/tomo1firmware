#ifndef FALL_DETECTION_H
#define FALL_DETECTION_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Configuration (defaults can be overridden at compile time with -D macros).
 *
 * A fall is detected when:
 *  1) Acceleration magnitude spikes above FD_IMPACT_G
 *  2) Followed within FD_SETTLE_MS by a quiet/low-motion period below FD_QUIET_G
 */
#ifndef FD_IMPACT_G
#define FD_IMPACT_G        2.5f   // g threshold for impact spike
#endif

#ifndef FD_QUIET_G
#define FD_QUIET_G         0.5f   // g threshold for post-impact quiet (stillness)
#endif

#ifndef FD_SETTLE_MS
#define FD_SETTLE_MS       800    // ms window after impact to observe quiet period
#endif

#ifndef FD_SAMPLE_PERIOD_MS
#define FD_SAMPLE_PERIOD_MS 50    // recommended polling period for fall_detection_check()
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t ax, ay, az;     // raw accel (LSB)
    int16_t gx, gy, gz;     // raw gyro (LSB)
    float   a_mag_g;        // accel magnitude in g
    uint32_t ts_ms;         // event timestamp (ms)
} fall_event_t;

/**
 * Initialize the MPU6050 and internal detector state.
 * Returns true on success, false on failure (sensor not found / config error).
 */
bool fall_detection_init(void);

/**
 * Poll once: reads sensor, updates detector, logs events.
 * Call this periodically (e.g., every FD_SAMPLE_PERIOD_MS via timer/task),
 * or from diagnostics while validating on bench.
 */
void fall_detection_check(void);

/**
 * Optional: get last detected event (returns true if one exists since boot).
 */
bool fall_detection_get_last(fall_event_t *out);

/**
 * Optional: override thresholds at runtime.
 */
void fall_detection_set_thresholds(float impact_g, float quiet_g, uint32_t settle_ms);

#ifdef __cplusplus
}
#endif

#endif // FALL_DETECTION_H
