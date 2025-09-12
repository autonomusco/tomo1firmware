#ifndef I2C_BUS_H
#define I2C_BUS_H
#include "driver/i2c.h"
void i2c_master_init(void);
esp_err_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len);
esp_err_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data);
#endif
