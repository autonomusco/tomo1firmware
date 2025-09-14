#include "fall_detection.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/i2c.h"
#include <math.h>
#include <string.h>

#define TAG "FALL_DETECTION"

// I2C settings
#ifndef FD_I2C_PORT
#define FD_I2C_PORT I2C_NUM_0
#endif

#ifndef FD_MPU_ADDR
#define FD_MPU_ADDR 0x68  // AD0 low=0x68, AD0 high=0x69
#endif

// MPU6050 registers
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_SMPLRT_DIV   0x19
#define MPU6050_REG_CONFIG       0x1A
#define MPU6050_REG_GYRO_CONFIG  0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_INT_ENABLE   0x38
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_WHO_AM_I     0x75

#define ACCEL_LSB_PER_G 16384.0f

// Internal detector state
typedef enum {
    FD_STATE_IDLE = 0,
    FD_STATE_IMPACT_OBSERVED
} fd_state_e;

typedef struct {
    float impact_g;
    float quiet_g;
    uint32_t settle_ms;

    fd_state_e state;
    uint32_t impact_ts_ms;

    fall_event_t last_evt;
    bool has_evt;
} fd_ctx_t;

static fd_ctx_t s_fd = {
    .impact_g = FD_IMPACT_G,
    .quiet_g  = FD_QUIET_G,
    .settle_ms = FD_SETTLE_MS,
    .state = FD_STATE_IDLE,
    .impact_ts_ms = 0,
    .has_evt = false
};

// Utility
static inline uint32_t now_ms(void) {
    return (uint32_t)(esp_timer_get_time() / 1000ULL);
}

static esp_err_t i2c_write_u8(uint8_t reg, uint8_t val) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FD_MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, val, true);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(FD_I2C_PORT, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);
    return err;
}

static esp_err_t i2c_read_bytes(uint8_t reg, uint8_t *buf, size_t len) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FD_MPU_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FD_MPU_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, buf, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, buf + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(FD_I2C_PORT, cmd, pdMS_TO_TICKS(200));
    i2c_cmd_link_delete(cmd);
    return err;
}

static bool mpu6050_whoami_ok(void) {
    uint8_t v = 0;
    if (i2c_read_bytes(MPU6050_REG_WHO_AM_I, &v, 1) != ESP_OK) {
        return false;
    }
    return (v == 0x68 || v == 0x69);
}

static bool mpu6050_init(void) {
    if (i2c_write_u8(MPU6050_REG_PWR_MGMT_1, 0x00) != ESP_OK) return false;
    if (i2c_write_u8(MPU6050_REG_SMPLRT_DIV, 9) != ESP_OK) return false;
    if (i2c_write_u8(MPU6050_REG_CONFIG, 0x03) != ESP_OK) return false;
    if (i2c_write_u8(MPU6050_REG_GYRO_CONFIG, 0x00) != ESP_OK) return false;
    if (i2c_write_u8(MPU6050_REG_ACCEL_CONFIG, 0x00) != ESP_OK) return false;
    if (i2c_write_u8(MPU6050_REG_INT_ENABLE, 0x00) != ESP_OK) return false;
    return true;
}

static bool mpu6050_read_raw(int16_t *ax, int16_t *ay, int16_t *az,
                             int16_t *gx, int16_t *gy, int16_t *gz) {
    uint8_t data[14];
    if (i2c_read_bytes(MPU6050_REG_ACCEL_XOUT_H, data, sizeof(data)) != ESP_OK) {
        return false;
    }
    *ax = (int16_t)((data[0] << 8) | data[1]);
    *ay = (int16_t)((data[2] << 8) | data[3]);
    *az = (int16_t)((data[4] << 8) | data[5]);
    *gx = (int16_t)((data[8] << 8) | data[9]);
    *gy = (int16_t)((data[10] << 8) | data[11]);
    *gz = (int16_t)((data[12] << 8) | data[13]);
    return true;
}

static float accel_mag_g_from_raw(int16_t ax, int16_t ay, int16_t az) {
    const float x = ax / ACCEL_LSB_PER_G;
    const float y = ay / ACCEL_LSB_PER_G;
    const float z = az / ACCEL_LSB_PER_G;
    return sqrtf(x*x + y*y + z*z);
}

// -------- PUBLIC API --------
bool fall_detection_init(void) {
    ESP_LOGI(TAG, "Initializing fall detection…");

    if (!mpu6050_whoami_ok()) {
        ESP_LOGE(TAG, "MPU6050 WHO_AM_I check failed");
        return false;
    }

    if (!mpu6050_init()) {
        ESP_LOGE(TAG, "MPU6050 configuration failed");
        return false;
    }

    memset(&s_fd.last_evt, 0, sizeof(s_fd.last_evt));
    s_fd.has_evt = false;
    s_fd.state = FD_STATE_IDLE;
    s_fd.impact_ts_ms = 0;

    ESP_LOGI(TAG, "Fall detection ready");
    return true;
}

void fall_detection_check(void) {
    int16_t ax, ay, az, gx, gy, gz;
    if (!mpu6050_read_raw(&ax, &ay, &az, &gx, &gy, &gz)) {
        ESP_LOGW(TAG, "Sensor read failed");
        return;
    }

    float a_mag_g = accel_mag_g_from_raw(ax, ay, az);
    uint32_t t_ms = now_ms();

    switch (s_fd.state) {
        case FD_STATE_IDLE:
            if (a_mag_g >= s_fd.impact_g) {
                s_fd.state = FD_STATE_IMPACT_OBSERVED;
                s_fd.impact_ts_ms = t_ms;
                ESP_LOGI(TAG, "Impact observed |a|=%.2fg", a_mag_g);
            }
            break;

        case FD_STATE_IMPACT_OBSERVED: {
            uint32_t dt = t_ms - s_fd.impact_ts_ms;

            if (a_mag_g <= s_fd.quiet_g && dt <= s_fd.settle_ms) {
                s_fd.state = FD_STATE_IDLE;

                s_fd.last_evt.ax = ax; s_fd.last_evt.ay = ay; s_fd.last_evt.az = az;
                s_fd.last_evt.gx = gx; s_fd.last_evt.gy = gy; s_fd.last_evt.gz = gz;
                s_fd.last_evt.a_mag_g = a_mag_g;
                s_fd.last_evt.ts_ms = t_ms;
                s_fd.has_evt = true;

                ESP_LOGW(TAG, "FALL DETECTED | |a|=%.2fg dt=%ums",
                         a_mag_g, (unsigned)dt);
            }

            if (dt > s_fd.settle_ms) {
                s_fd.state = FD_STATE_IDLE;
                ESP_LOGI(TAG, "Impact timeout");
            }
            break;
        }
        default:
            s_fd.state = FD_STATE_IDLE;
            break;
    }
}

bool fall_detection_get_last(fall_event_t *out) {
    if (!s_fd.has_evt || out == NULL) return false;
    *out = s_fd.last_evt;
    return true;
}

void fall_detection_set_thresholds(float impact_g, float quiet_g, uint32_t settle_ms) {
    if (impact_g > 0.3f) s_fd.impact_g = impact_g;
    if (quiet_g  > 0.05f) s_fd.quiet_g  = quiet_g;
    if (settle_ms >= 100) s_fd.settle_ms = settle_ms;

    ESP_LOGI(TAG, "Thresholds updated (impact_g=%.2fg, quiet_g=%.2fg, settle=%ums)",
             s_fd.impact_g, s_fd.quiet_g, (unsigned)s_fd.settle_ms);
}
