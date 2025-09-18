#include "i2c_bus.h"
#include "esp_log.h"

#define I2C_PORT I2C_NUM_0
static const char *TAG = "I2C";

esp_err_t i2c_bus_init(gpio_num_t sda, gpio_num_t scl, uint32_t freq_hz) {
    i2c_config_t cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda, .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl, .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq_hz
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &cfg));
    return i2c_driver_install(I2C_PORT, cfg.mode, 0, 0, 0);
}

esp_err_t i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_PORT, addr, &reg, 1, data, len, 1000 / portTICK_PERIOD_MS);
}

esp_err_t i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len) {
    uint8_t buf[1 + len];
    buf[0] = reg;
    for (size_t i=0;i<len;i++) buf[1+i] = data[i];
    return i2c_master_write_to_device(I2C_PORT, addr, buf, sizeof(buf), 1000 / portTICK_PERIOD_MS);
}
