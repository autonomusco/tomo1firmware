#include "power_mgmt.h"
#include "i2c_bus.h"
#include "esp_log.h"

#define MAX17048_ADDR 0x36
#define REG_SOC 0x04

static const char *TAG = "POWER/MAX17048";

void power_init(void) {
    ESP_LOGI(TAG, "MAX17048 init (nothing required for basic mode)");
}

int power_get_battery_percent(void) {
    uint8_t buf[2];
    if (i2c_read_bytes(MAX17048_ADDR, REG_SOC, buf, 2) == ESP_OK) {
        int soc = buf[0];
        ESP_LOGI(TAG, "Battery SOC=%d%%", soc);
        return soc;
    }
    return -1;
}
