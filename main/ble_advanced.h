#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * Advanced BLE characteristics for configuration and OTA triggers.
 * Requires bonded connection.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize advanced BLE characteristics (called after ble_init). */
void ble_advanced_init(void);

/** Apply a new device name from BLE config characteristic. */
bool ble_advanced_set_device_name(const char *name);

/** Start OTA update (triggered by BLE characteristic). */
bool ble_advanced_trigger_ota(void);

#ifdef __cplusplus
}
#endif
