#pragma once

/**
 * @file cloud_api.h
 * @brief Cloud API stubs for telemetry and alert reporting.
 *
 * Provides functions to initialize the cloud subsystem,
 * send telemetry packets, and send alerts.
 * 
 * Stage 7: Stub implementation (logs only).
 * Future stages: Integrate with HTTP/MQTT for real cloud comms.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize cloud API subsystem.
 */
void cloud_api_init(void);

/**
 * @brief Send telemetry data to the cloud.
 *
 * @param voltage Battery voltage in volts.
 * @param soc     State of charge (%).
 * @param ax      Acceleration X (g).
 * @param ay      Acceleration Y (g).
 * @param az      Acceleration Z (g).
 */
void cloud_api_send_telemetry(float voltage, float soc,
                              float ax, float ay, float az);

/**
 * @brief Send alert event to the cloud.
 *
 * @param code Alert code identifier.
 */
void cloud_api_send_alert(uint8_t code);

#ifdef __cplusplus
}
#endif
