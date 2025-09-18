#pragma once
#include "driver/i2c.h"
esp_err_t i2c_bus_init(gpio_num_t sda, gpio_num_t scl, uint32_t freq_hz);
esp_err_t i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len);
