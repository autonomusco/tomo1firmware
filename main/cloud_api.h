#pragma once
#include <stdbool.h>
#include "esp_err.h"

/**
 * Cloud API Hooks
 * Stage 7: Stub functions to send telemetry and alerts.
 * Future: implement esp_http_client with real endpoint + TLS.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize cloud API (stub). */
void cloud_api_init(void);

/** Send telemetry data upstream (stub). */
bool cloud_api_send_telemetry(float voltage, float soc,
                              float ax, float ay, float az);

/** Send an alert code upstream (stub). */
bool cloud_api_send_alert(uint8_t code);

#ifdef __cplusplus
}
#endif
