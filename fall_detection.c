#include "fall_detection.h"
#include "i2c_bus.h"
#include "esp_log.h"

#define MPU6050_ADDR 0x68
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B

static const char *TAG = "FALL/MPU6050";

void fall_sensor_init(void) {
    ESP_LOGI(TAG, "Initializing MPU6050...");
    i2c_write_byte(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, 0x00);
}

bool fall_detected(void) {
    uint8_t buf[6];
    if (i2c_read_bytes(MPU6050_ADDR, MPU6050_REG_ACCEL_XOUT_H, buf, 6) == ESP_OK) {
        int16_t ax = (buf[0]<<8)|buf[1];
        int16_t ay = (buf[2]<<8)|buf[3];
        int16_t az = (buf[4]<<8)|buf[5];
        ESP_LOGI(TAG, "Accel: ax=%d ay=%d az=%d", ax, ay, az);
    }
    return false;
}

void fall_task_start(void) {}
