#include "i2c_bus.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 9
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TIMEOUT_MS 1000

static const char *TAG = "I2C_BUS";

void i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    ESP_LOGI(TAG, "I2C initialized on SDA=%d, SCL=%d", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
}

esp_err_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len) {
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);
}

esp_err_t i2c_write_byte(uint8_t dev_addr, uint8_t reg_addr, uint8_t data) {
    uint8_t buf[2] = { reg_addr, data };
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, buf, 2, I2C_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);
}
