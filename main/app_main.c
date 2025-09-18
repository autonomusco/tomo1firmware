#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "i2c_bus.h"
#include "max17048.h"
#include "mpu6050.h"
#include "button.h"
#include "ble.h"

static const char *TAG = "APP";
static esp_timer_handle_t telemetry_timer;

/* --------- Button Callback --------- */
static void alert_cb(button_event_t ev) {
    if (ev == BUTTON_EVENT_PRESS) {
        ESP_LOGW(TAG, "ALERT button pressed");
        ble_send_alert_code(0x01);   // 0x01 = button press
    } else if (ev == BUTTON_EVENT_LONG) {
        ESP_LOGW(TAG, "LONG button press");
        ble_send_alert_code(0x11);   // custom long-press code
    }
}

/* --------- Telemetry Timer --------- */
static void telemetry_cb(void *arg) {
    float soc = max17048_get_soc();
    mpu6050_reading_t m = mpu6050_read();

    if (soc >= 0.f) {
        ble_update_battery((uint8_t)(soc + 0.5f));
    }
    ble_update_motion(m.ax, m.ay, m.az);

    if (mpu6050_fall_detected()) {
        ESP_LOGW(TAG, "FALL detected -> sending alert");
        ble_send_alert_code(0x02);   // 0x02 = fall event
    }

    ESP_LOGI(TAG, "SOC: %.1f%% | accel[g]: [%.2f, %.2f, %.2f]",
             soc, m.ax, m.ay, m.az);
}

/* --------- Main Entry --------- */
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    // Init I²C bus on SDA=8, SCL=9, 400kHz
    ESP_ERROR_CHECK(i2c_bus_init(GPIO_NUM_8, GPIO_NUM_9, 400000));

    // Init sensors
    max17048_init();
    mpu6050_init();

    // Init button on GPIO0
    button_init(GPIO_NUM_0, alert_cb);

    // Init BLE
    ble_init();
    ble_set_device_name("TomoPendant");
    ble_start_advertising();

    // Start telemetry timer (5s period)
    const esp_timer_create_args_t tcfg = {
        .callback = telemetry_cb,
        .name = "telemetry"
    };
    ESP_ERROR_CHECK(esp_timer_create(&tcfg, &telemetry_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(telemetry_timer, 5 * 1000 * 1000));
}
