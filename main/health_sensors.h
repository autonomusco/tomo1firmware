#pragma once

typedef struct {
    float spo2;   // Oxygen saturation (%)
    float hr;     // Heart rate (bpm)
} health_reading_t;

/**
 * Initialize health sensors (stub or real).
 */
void health_sensors_init(void);

/**
 * Read health sensor values.
 * @return struct containing SpO2 and HR
 */
health_reading_t health_sensors_read(void);
