#include "power_mgmt.h"
#include "esp_log.h"
#include "driver/i2c.h"

#define TAG "POWER_MGMT"

#ifndef PM_I2C_PORT
#define PM_I2C_PORT I2C_NUM_0
#endif

#ifndef PM_MAX17048_ADDR
#define PM_MAX17048_ADDR 0x36
#endif

// MAX17048 registers
#define MAX17048_REG_VCELL   0x02
#define MAX17048_REG_SOC     0x04
#define MAX17048_REG_VERSION 0x08

// Helpers
static esp_err_t i2c_read16(uint8_t reg, uint16_t *out) {
    uint8_t data[2];
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PM_MAX17048_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (PM_MAX17048_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, 1, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, data + 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(PM_I2C_PORT, cmd, pdMS_TO_TICKS(200));
    i2c_cmd_link_delete(cmd);
    if (err != ESP_OK) return err;
    *out = ((uint16_t)data[0] << 8) | data[1];
    return ESP_OK;
}

bool power_mgmt_init(void) {
    uint16_t version = 0;
    if (i2c_read16(MAX17048_REG_VERSION, &version) != ESP_OK) {
        ESP_LOGE(TAG, "MAX17048 not responding");
        return false;
    }
    ESP_LOGI(TAG, "MAX17048 detected, version=0x%04X", version);
    return true;
}

bool power_mgmt_get_voltage(float *voltage) {
    if (!voltage) return false;
    uint16_t raw = 0;
    if (i2c_read16(MAX17048_REG_VCELL, &raw) != ESP_OK) return false;
    *voltage = (raw >> 4) * 0.00125f; // 1.25mV per LSB
    return true;
}

bool power_mgmt_get_soc(float *soc) {
    if (!soc) return false;
    uint16_t raw = 0;
    if (i2c_read16(MAX17048_REG_SOC, &raw) != ESP_OK) return false;
    *soc = (raw >> 8) + ((raw & 0xFF) / 256.0f);
    return true;
}

