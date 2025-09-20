#pragma once
#include <stddef.h>   // ✅ ensures NULL is defined

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Record or log a diagnostic event.
 *
 * Example:
 *   diagnostics_event("System init complete", NULL);
 *   diagnostics_log_event("Battery low");
 */
void diagnostics_event(const char *msg, const char *context);

/**
 * Alias for simple logging without context.
 * Equivalent to diagnostics_event(msg, NULL).
 */
static inline void diagnostics_log_event(const char *msg) {
    diagnostics_event(msg, NULL);
}

#ifdef __cplusplus
}
#endif
