#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Stage 10 – Hardening & Watchdog Baseline
 *
 * Thin wrapper around ESP-IDF Task Watchdog (TWDT).
 * - Does NOT force resets in CI; safe to link & call in builds without hardware.
 * - Add/remove current task, feed periodically, simple self-test hook.
 */

void watchdog_init(void);

/** Register the current task with the TWDT. Safe to call multiple times. */
bool watchdog_enable_task(void);

/** Unregister the current task from the TWDT. */
bool watchdog_disable_task(void);

/** Feed the watchdog for the current task. */
bool watchdog_feed(void);

/** CI-friendly smoke test to prove symbols compile & link. */
void watchdog_self_test(void);

#ifdef __cplusplus
}
#endif
