#include "ble_advanced.h"
#include "ble.h"
#include "ota_update.h"
#include "esp_log.h"
#include "host/ble_hs.h"

static const char *TAG = "BLE_ADV";

static uint16_t s_cfg_val_handle = 0;
static uint16_t s_ota_val_handle = 0;

/* UUIDs */
static const ble_uuid128_t CFG_UUID =
    BLE_UUID128_INIT(0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x00);
static const ble_uuid128_t OTA_UUID =
    BLE_UUID128_INIT(0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x01);

static int gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                          struct ble_gatt_access_ctxt *ctxt, void *arg) {
    (void)conn_handle;
    (void)arg;

    if (attr_handle == s_cfg_val_handle) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            char buf[32];
            int len = ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf)-1, NULL);
            if (len > 0) {
                buf[len] = '\0';
                ble_advanced_set_device_name(buf);
            }
        }
        return 0;
    }

    if (attr_handle == s_ota_val_handle) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            ESP_LOGW(TAG, "BLE OTA trigger received");
            ble_advanced_trigger_ota();
        }
        return 0;
    }

    return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
}

void ble_advanced_init(void) {
    static const struct ble_gatt_chr_def adv_chrs[] = {
        {
            .uuid = &CFG_UUID.u,
            .access_cb = gatt_access_cb,
            .val_handle = &s_cfg_val_handle,
            .flags = BLE_GATT_CHR_F_WRITE,
        },
        {
            .uuid = &OTA_UUID.u,
            .access_cb = gatt_access_cb,
            .val_handle = &s_ota_val_handle,
            .flags = BLE_GATT_CHR_F_WRITE,
        },
        { 0 }
    };

    static const struct ble_gatt_svc_def adv_svc[] = {
        {
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = BLE_UUID128_DECLARE(0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x11,0x22,
                                        0x33,0x44,0x55,0x66,0x77,0x88,0x99,0x02),
            .characteristics = adv_chrs,
        },
        { 0 }
    };

    int rc = ble_gatts_count_cfg(adv_svc);
    if (!rc) rc = ble_gatts_add_svcs(adv_svc);
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to add advanced BLE services rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Advanced BLE services added");
    }
}

bool ble_advanced_set_device_name(const char *name) {
    if (!name || !name[0]) return false;
    ESP_LOGI(TAG, "Setting device name to: %s", name);
    ble_set_device_name(name);
    return true;
}

bool ble_advanced_trigger_ota(void) {
    ESP_LOGI(TAG, "Triggering OTA update...");
    ota_update_start();
    return true;
}
