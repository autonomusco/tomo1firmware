#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * Advanced BLE characteristics for configuration and OTA triggers.
 * 
 * Service UUID (128-bit):
 *   0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22,
 *   0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x02
 *
 * Characteristics:
 *   - Config (device name, thresholds)
 *       UUID: ...0x00
 *   - OTA trigger
 *       UUID: ...0x01
 *
 * NOTE: Only available after pairing/bonding (security enforced at GAP level).
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the Advanced BLE service (called after ble_pairing_init). */
void ble_advanced_init(void);

/** Apply a new device name (called internally by BLE config write). */
bool ble_advanced_set_device_name(const char *name);

/** Start OTA update (called internally by BLE OTA trigger write). */
bool ble_advanced_trigger_ota(void);

#ifdef __cplusplus
}
#endif
