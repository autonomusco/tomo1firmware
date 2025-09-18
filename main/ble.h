#pragma once
#include <stdint.h>
void ble_init(void);
void ble_set_device_name(const char *name);
void ble_start_advertising(void);
void ble_update_battery(uint8_t percent);
void ble_update_motion(float ax, float ay, float az);
void ble_send_alert(void);
