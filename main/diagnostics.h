#pragma once
#include <stdbool.h>
#include "esp_err.h"

/**
 * Lightweight diagnostics/logging facade.
 * Today: logs to ESP_LOG with timestamps.
 * Later: can be extended to persist, upload to cloud, etc.
 */

#ifdef __cplusplus
extern "C" {
#endif

void diagnostics_init(void);

/** Log a simple event with optional details (safe to pass NULL). */
void diagnostics_event(const char *event_name, const char *detail);

/** Log an error from a subsystem/location with esp_err_t code. */
void diagnostics_error(const char *where, esp_err_t err, const char *detail);

#ifdef __cplusplus
}
#endif
