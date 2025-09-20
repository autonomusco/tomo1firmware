#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Record or log a diagnostic event.
 * Optionally attach context data (string, JSON, etc.).
 *
 * Example:
 *   diagnostics_event("System init complete", NULL);
 *   diagnostics_log_event("Battery low");
 */

void diagnostics_event(const char *msg, const char *context);

// Alias for legacy calls
static inline void diagnostics_log_event(const char *msg) {
    diagnostics_event(msg, NULL);
}

#ifdef __cplusplus
}
#endif
