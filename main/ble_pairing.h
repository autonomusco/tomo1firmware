#pragma once

/**
 * @file ble_pairing.h
 * @brief BLE pairing subsystem API for Tomo Pendant.
 *
 * Provides initialization and cleanup functions for
 * the NimBLE host stack, advertising, and pairing.
 */

/**
 * @brief Initialize BLE pairing subsystem.
 *
 * Starts the NimBLE host stack and advertising.
 * Uses ESP-IDF v5.2 NimBLE lifecycle:
 *   - esp_nimble_hci_init()
 *   - nimble_port_init()
 *   - xTaskCreate(nimble_port_run, ...)
 */
void ble_pairing_init(void);

/**
 * @brief Deinitialize BLE pairing subsystem.
 *
 * Stops the NimBLE host stack and cleans up resources.
 * Uses:
 *   - nimble_port_stop()
 *   - nimble_port_deinit()
 */
void ble_pairing_deinit(void);
