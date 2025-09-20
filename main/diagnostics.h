#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Log a diagnostic event.
 * Example: diagnostics_log_event("System init complete");
 */
void diagnostics_log_event(const char *msg);

#ifdef __cplusplus
}
#endif
