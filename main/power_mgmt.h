#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Initialize the battery monitor (MAX17048).
 * Returns true if device responds, false otherwise.
 */
bool power_mgmt_init(void);

/**
 * Read current battery voltage (in volts).
 * Returns true if successful, false if sensor not responding.
 */
bool power_mgmt_get_voltage(float *voltage);

/**
 * Read current state-of-charge (SOC, %).
 * Returns true if successful, false if sensor not responding.
 */
bool power_mgmt_get_soc(float *soc);

#endif // POWER_MGMT_H
