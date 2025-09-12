#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "i2c_bus.h"
#include "ble_pairing.h"
#include "fall_detection.h"
#include "emergency_button.h"
#include "cloud_api.h"
#include "ota_update.h"
#include "power_mgmt.h"
#include "health_sensors.h"
#include "voice_trigger.h"
#include "diagnostics.h"
#include "security.h"

static const char *TAG = "TOMO_MAIN";

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "Booting Tomo Pendant Firmware (HW stubbed)");

    i2c_master_init();
    power_init();
    fall_sensor_init();

    ble_init();
    ble_start_advertising();
    cloud_api_init();
    emergency_button_init();
    health_sensors_init();
    voice_trigger_init();
    diag_init();
    security_init();
    ota_init();

    while (true) {
        int soc = power_get_battery_percent();
        ESP_LOGI(TAG, "Heartbeat. Battery=%d%%", soc);
        fall_detected();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
