#ifndef BLE_PAIRING_H
#define BLE_PAIRING_H

#include <stdbool.h>

// Initialize BLE pairing
bool ble_pairing_init(void);

// Update battery state of charge over BLE
void ble_pairing_update_battery(int soc);

#endif // BLE_PAIRING_H
