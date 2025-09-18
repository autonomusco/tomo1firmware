#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ble_init(void);
void ble_set_device_name(const char *name);
void ble_start_advertising(void);

void ble_update_battery(uint8_t percent);     // 0..100
void ble_update_motion(float ax, float ay, float az); // reserved for future payloads
void ble_send_alert(void);                    // notify alert characteristic (e.g., button/fall)

#ifdef __cplusplus
}
#endif
