#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "i2c_bus.h"
#include "power_mgmt.h"
#include "mpu6050.h"
#include "button.h"
#include "ble.h"

static const char *TAG = "APP";
static esp_timer_handle_t telemetry_timer;

/* --------- Button Callback --------- */
static void alert_cb(button_event_t ev) {
    if (ev == BUTTON_EVENT_PRESS) {
        ESP_LOGW(TAG, "ALERT button pressed");
        ble_send_alert_code(0x01);
    } else if (ev == BUTTON_EVENT_LONG) {
        ESP_LOGW(TAG, "LONG button press");
        ble_send_alert_code(0x11);
    }
}

/* --------- Telemetry Timer --------- */
static void telemetry_cb(void *arg) {
    float voltage = 0.0f;
    float soc = 0.0f;

    if (power_mgmt_get_voltage(&voltage)) {
        ESP_LOGI(TAG, "Voltage: %.2f V", voltage);
    }

    if (power_mgmt_get_soc(&soc)) {
        ble_update_battery((uint8_t)(soc + 0.5f));
        ESP_LOGI(TAG, "SOC: %.1f%%", soc);
    }

    mpu6050_reading_t m = mpu6050_read();
    ble_update_motion(m.ax, m.ay, m.az);

    if (mpu6050_fall_detected()) {
        ESP_LOGW(TAG, "FALL detected -> sending alert");
        ble_send_alert_code(0x02);
    }

    ESP_LOGI(TAG, "Telemetry | Vbat=%.2fV SOC=%.1f%% | accel[g]=[%.2f, %.2f, %.2f]",
             voltage, soc, m.ax, m.ay, m.az);
}

/* --------- Main Entry --------- */
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    // Init I²C bus
    ESP_ERROR_CHECK(i2c_bus_init(GPIO_NUM_8, GPIO_NUM_9, 400000));

    // Init subsystems
    power_mgmt_init();
    mpu6050_init();
    button_init(GPIO_NUM_0, alert_cb);

    // Init BLE
    ble_init();
    ble_set_device_name("TomoPendant");
    ble_start_advertising();

    // Start telemetry timer (5s)
    const esp_timer_create_args_t tcfg = {
        .callback = telemetry_cb,
        .name = "telemetry"
    };
    ESP_ERROR_CHECK(esp_timer_create(&tcfg, &telemetry_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(telemetry_timer, 5 * 1000 * 1000));
}
