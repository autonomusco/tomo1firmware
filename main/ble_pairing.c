#include "ble_pairing.h"
#include "esp_log.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define TAG "BLE_PAIRING"

static uint16_t battery_handle;
static uint16_t fall_handle;
static uint16_t button_handle;

// GATT characteristics
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180F), // Battery Service UUID
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID16_DECLARE(0x2A19), // Battery Level
                .access_cb = NULL,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &battery_handle,
            },
            {0} // terminator
        },
    },
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID128_DECLARE(0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF,
                                    0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF),
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = BLE_UUID128_DECLARE(0xFA,0x11,0x00,0x00,0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01),
                .access_cb = NULL,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &fall_handle,
            },
            {
                .uuid = BLE_UUID128_DECLARE(0xFA,0x22,0x00,0x00,0x00,0x00,0x00,0x00,
                                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02),
                .access_cb = NULL,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &button_handle,
            },
            {0} // terminator
        },
    },
    {0}, // terminator
};

static int ble_gap_event(struct ble_gap_event *event, void *arg) {
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                ESP_LOGI(TAG, "Connected");
            } else {
                ESP_LOGI(TAG, "Connection failed; retrying");
                ble_gap_adv_start(0, NULL, BLE_HS_FOREVER,
                                  &(struct ble_gap_adv_params){0}, ble_gap_event, NULL);
            }
            break;
        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "Disconnected; restarting advertising");
            ble_gap_adv_start(0, NULL, BLE_HS_FOREVER,
                              &(struct ble_gap_adv_params){0}, ble_gap_event, NULL);
            break;
        default:
            break;
    }
    return 0;
}

static void ble_on_sync(void) {
    ble_hs_id_infer_auto(0, NULL);
    struct ble_gap_adv_params adv_params = {0};
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    int rc = ble_gap_adv_start(0, NULL, BLE_HS_FOREVER,
                               &adv_params, ble_gap_event, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "Advertising start failed: %d", rc);
    } else {
        ESP_LOGI(TAG, "Advertising as TomoPendant");
    }
}

bool ble_pairing_init(void) {
    ESP_LOGI(TAG, "Initializing BLE…");

    // Init NimBLE
    nimble_port_init();

    // Register services
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    // Sync callback
    ble_hs_cfg.sync_cb = ble_on_sync;

    // Start host thread
    nimble_port_freertos_init(ble_hs_task);

    ESP_LOGI(TAG, "BLE init complete");
    return true;
}

void ble_pairing_update_battery(float soc_percent) {
    uint8_t val = (uint8_t)(soc_percent + 0.5f);
    ble_gatts_chr_updated(battery_handle);
    ble_gatts_set_attr(battery_handle, &val, sizeof(val));
}

void ble_pairing_update_fall(int fall_flag) {
    uint8_t val = (fall_flag ? 1 : 0);
    ble_gatts_chr_updated(fall_handle);
    ble_gatts_set_attr(fall_handle, &val, sizeof(val));
}

void ble_pairing_update_button(int button_flag) {
    uint8_t val = (button_flag ? 1 : 0);
    ble_gatts_chr_updated(button_handle);
    ble_gatts_set_attr(button_handle, &val, sizeof(val));
}