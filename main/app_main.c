#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_sleep.h"

#include "i2c_bus.h"
#include "power_mgmt.h"
#include "mpu6050.h"
#include "fall_detection.h"
#include "button.h"
#include "emergency_button.h"
#include "ble.h"
#include "ota_update.h"
#include "diagnostics.h"
#include "cloud_api.h"

static const char *TAG = "APP";
static esp_timer_handle_t telemetry_timer;

/* --------- Normal Button Callback --------- */
static void alert_cb(button_event_t ev) {
    if (ev == BUTTON_EVENT_PRESS) {
        diagnostics_event("button_press", "normal");
        ESP_LOGW(TAG, "ALERT button pressed");
        ble_send_alert_code(0x01);
        cloud_api_send_alert(0x01);
    } else if (ev == BUTTON_EVENT_LONG) {
        diagnostics_event("button_long", "normal");
        ESP_LOGW(TAG, "LONG button press");
        ble_send_alert_code(0x11);
        cloud_api_send_alert(0x11);
    }
}

/* --------- Emergency Button Callback --------- */
static void emergency_cb(emergency_event_t ev) {
    if (ev == EMERGENCY_EVENT_PRESS) {
        diagnostics_event("button_press", "emergency");
        ESP_LOGW(TAG, "EMERGENCY button pressed");
        ble_send_alert_code(0x03);
        cloud_api_send_alert(0x03);
    } else if (ev == EMERGENCY_EVENT_LONG) {
        diagnostics_event("button_long", "emergency");
        ESP_LOGW(TAG, "EMERGENCY button long press");
        ble_send_alert_code(0x13);
        cloud_api_send_alert(0x13);

        // Example future hook: trigger OTA update (stub)
        ota_update_start();
    }
}

/* --------- Telemetry Timer --------- */
static void telemetry_cb(void *arg) {
    float voltage = 0.0f;
    float soc = 0.0f;

    if (power_mgmt_get_voltage(&voltage)) {
        char msg[48];
        snprintf(msg, sizeof(msg), "V=%.2fV", voltage);
        diagnostics_event("battery_voltage", msg);
        ESP_LOGI(TAG, "Voltage: %.2f V", voltage);
    }

    if (power_mgmt_get_soc(&soc)) {
        char msg[48];
        snprintf(msg, sizeof(msg), "SOC=%.1f%%", soc);
        diagnostics_event("battery_soc", msg);
        ble_update_battery((uint8_t)(soc + 0.5f));
        ESP_LOGI(TAG, "SOC: %.1f%%", soc);
    }

    mpu6050_reading_t m = mpu6050_read();
    ble_update_motion(m.ax, m.ay, m.az);

    if (fall_detection_check(&m)) {
        diagnostics_event("fall_detected", NULL);
        ESP_LOGW(TAG, "FALL detected -> sending alert");
        ble_send_alert_code(0x02);
        cloud_api_send_alert(0x02);
    }

    ESP_LOGI(TAG, "Telemetry | Vbat=%.2fV SOC=%.1f%% | accel[g]=[%.2f, %.2f, %.2f]",
             voltage, soc, m.ax, m.ay, m.az);

    cloud_api_send_telemetry(voltage, soc, m.ax, m.ay, m.az);
}

/* --------- Main Entry --------- */
void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    diagnostics_init();
    cloud_api_init();

    // Init I²C bus
    esp_err_t err = i2c_bus_init(GPIO_NUM_8, GPIO_NUM_9, 400000);
    if (err != ESP_OK) diagnostics_error("i2c_bus_init", err, "init failed");
    ESP_ERROR_CHECK(err);

    // Init subsystems
    power_mgmt_init();
    mpu6050_init();
    fall_detection_init();
    button_init(GPIO_NUM_0, alert_cb);
    emergency_button_init(GPIO_NUM_1, emergency_cb);
    ota_update_init();

    // Init BLE
    ble_init();
    ble_set_device_name("TomoPendant");
    diagnostics_event("ble_advertising_start", "name=TomoPendant");
    ble_start_advertising();

    // Start telemetry timer (5s)
    const esp_timer_create_args_t tcfg = {
        .callback = telemetry_cb,
        .name = "telemetry"
    };
    ESP_ERROR_CHECK(esp_timer_create(&tcfg, &telemetry_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(telemetry_timer, 5 * 1000 * 1000));
}
