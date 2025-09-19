#pragma once
#include <stdbool.h>
#include "mpu6050.h"

/**
 * Initialize fall detection subsystem.
 * Currently a stub (no-op).
 */
void fall_detection_init(void);

/**
 * Check for fall event based on MPU6050 reading.
 * Returns true if fall detected, false otherwise.
 */
bool fall_detection_check(const mpu6050_reading_t *m);
