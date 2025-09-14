#ifndef BLE_PAIRING_H
#define BLE_PAIRING_H

#include <stdbool.h>   // for bool type

/**
 * @brief Initialize BLE pairing services.
 *
 * @return true if initialization succeeds, false otherwise.
 */
bool ble_pairing_init(void);

#endif // BLE_PAIRING_H