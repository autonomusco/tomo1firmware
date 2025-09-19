#pragma once
#include <stdbool.h>
#include "esp_err.h"

/**
 * Stage 8: Health sensor integration (stubs).
 * For now, returns dummy SpO₂ + HR values.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float spo2;   // oxygen saturation %
    float hr;     // heart rate bpm
} health_reading_t;

/** Initialize health sensor subsystem (stub). */
esp_err_t health_sensors_init(void);

/** Read health values (stub). */
health_reading_t health_sensors_read(void);

#ifdef __cplusplus
}
#endif
