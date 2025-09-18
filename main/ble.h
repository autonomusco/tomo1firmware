#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ble_init(void);
void ble_set_device_name(const char *name);
void ble_start_advertising(void);

/* Battery %, 0..100. Notifies if subscribed. */
void ble_update_battery(uint8_t percent);

/* Motion: ax/ay/az in g (float). We’ll pack & notify as int16 mg if subscribed. */
void ble_update_motion(float ax, float ay, float az);

/* Send alert code to caregiver app (e.g., 0x01=button, 0x02=fall). */
void ble_send_alert_code(uint8_t code);

/* Convenience retained for button press path (sends 0x01). */
static inline void ble_send_alert(void) { ble_send_alert_code(0x01); }

#ifdef __cplusplus
}
#endif
