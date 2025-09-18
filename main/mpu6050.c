#include "mpu6050.h"
#include "i2c_bus.h"
#include "esp_log.h"
#include <math.h>

#define MPU_ADDR 0x68
#define REG_PWR_MGMT_1 0x6B
#define REG_ACC_XOUT_H 0x3B

static const char *TAG = "MPU6050";
static bool ok = false;

bool mpu6050_init(void) {
    uint8_t zero = 0x00; // wake up device
    if (i2c_write_reg(MPU_ADDR, REG_PWR_MGMT_1, &zero, 1) != ESP_OK) return false;
    ok = true;
    return true;
}

mpu6050_reading_t mpu6050_read(void) {
    mpu6050_reading_t r = {0};
    uint8_t buf[14];
    if (!ok) return r;
    if (i2c_read_reg(MPU_ADDR, REG_ACC_XOUT_H, buf, sizeof(buf)) != ESP_OK) return r;

    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];
    int16_t gx = (buf[8] << 8) | buf[9];
    int16_t gy = (buf[10] << 8) | buf[11];
    int16_t gz = (buf[12] << 8) | buf[13];

    // Default ±2g, ±250 dps
    r.ax = ax / 16384.0f; r.ay = ay / 16384.0f; r.az = az / 16384.0f;
    r.gx = gx / 131.0f;   r.gy = gy / 131.0f;   r.gz = gz / 131.0f;
    return r;
}

bool mpu6050_fall_detected(void) {
    mpu6050_reading_t r = mpu6050_read();
    float amag = sqrtf(r.ax*r.ax + r.ay*r.ay + r.az*r.az);
    // crude: large spike then near-zero (free fall); refine later
    static float prev = 1.0f;
    bool spike = (amag > 2.2f);      // impact
    bool dip   = (prev > 1.5f && amag < 0.4f);
    prev = amag;
    return spike || dip;
}
