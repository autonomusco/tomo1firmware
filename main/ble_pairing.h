#ifndef BLE_PAIRING_H
#define BLE_PAIRING_H

#include <stdbool.h>

/**
 * Initialize BLE, start advertising, and set up GATT service.
 * Returns true if successful, false otherwise.
 */
bool ble_pairing_init(void);

/**
 * Update the battery percentage characteristic.
 */
void ble_pairing_update_battery(float soc_percent);

/**
 * Update the fall-detected flag characteristic (1 = fall, 0 = none).
 */
void ble_pairing_update_fall(int fall_flag);

/**
 * Update the emergency button pressed flag (1 = pressed, 0 = idle).
 */
void ble_pairing_update_button(int button_flag);

#endif // BLE_PAIRING_H