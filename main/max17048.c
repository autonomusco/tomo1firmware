#include "max17048.h"
#include "i2c_bus.h"
#include "esp_log.h"

#define MAX17048_ADDR 0x36
#define REG_VCELL     0x02
#define REG_SOC       0x04
#define REG_MODE      0x06
#define REG_VERSION   0x08

static const char *TAG = "MAX17048";
static bool ready = false;

static uint16_t rd16(uint8_t reg) {
    uint8_t d[2] = {0};
    if (i2c_read_reg(MAX17048_ADDR, reg, d, 2) != ESP_OK) return 0;
    return (d[0] << 8) | d[1];
}

void max17048_init(void) {
    uint16_t ver = rd16(REG_VERSION);
    if (ver) ready = true;
    ESP_LOGI(TAG, "version reg: 0x%04x", ver);
}

float max17048_get_voltage(void) {
    if (!ready) return -1.f;
    uint16_t raw = rd16(REG_VCELL);
    float volts = ((raw >> 4) * 1.25f) / 1000.0f; // 12-bit, 1.25mV/LSB
    return volts;
}

float max17048_get_soc(void) {
    if (!ready) return -1.f;
    uint16_t raw = rd16(REG_SOC);
    float soc = (raw >> 8) + ((raw & 0xFF) / 256.0f);

    if (soc < 0) {
        soc = 0;
    }
    if (soc > 100) {
        soc = 100;
    }

    return soc;
}
