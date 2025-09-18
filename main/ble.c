#include "ble.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "BLE";

static uint16_t batt_handle = 0;
static uint16_t alert_handle = 0;
static uint8_t adv_data[31];
static char dev_name[20] = "TomoPendant";

#define UUID16_BAS  0x180F
#define UUID16_BATT 0x2A19

// Tomo custom primary service UUID: 128-bit (example)
static const uint8_t TOMO_SVC_UUID[16] = { 0x9a,0x33,0x21,0x10,0x5c,0x0d,0x4a,0xb1,0x9a,0x22,0x55,0x66,0x11,0x22,0x33,0x44 };
static const uint8_t TOMO_ALERT_UUID[16] = {0x9a,0x33,0x21,0x10,0x5c,0x0d,0x4a,0xb1,0x9a,0x22,0x55,0x66,0x11,0x22,0x33,0x45};

static esp_gatt_if_t gatts_if_global = 0;
static uint16_t gatts_conn_id = 0;

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event == ESP_GAP_BLE_ADV_START_COMPLETE_EVT) {
        ESP_LOGI(TAG, "adv started");
    }
}

static void set_adv() {
    esp_ble_adv_params_t adv = {
        .adv_int_min = 0x40, .adv_int_max = 0x60,
        .adv_type = ADV_TYPE_IND, .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .channel_map = ADV_CHNL_ALL, .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };
    esp_ble_gap_start_advertising(&adv);
}

static void gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                     esp_ble_gatts_cb_param_t *param) {
    gatts_if_global = gatts_if;

    switch (event) {
    case ESP_GATTS_REG_EVT: {
        ESP_LOGI(TAG, "GATTS REG");
        esp_ble_gap_set_device_name(dev_name);
        // Battery Service
        esp_gatt_srvc_id_t bas = {
            .is_primary = true,
            .id.inst_id = 0,
            .id.uuid.len = ESP_UUID_LEN_16,
            .id.uuid.uuid.uuid16 = UUID16_BAS
        };
        esp_ble_gatts_create_service(gatts_if, &bas, 6);
        break;
    }
    case ESP_GATTS_CREATE_EVT: {
        uint16_t svc_handle = param->create.service_handle;
        esp_gatt_srvc_id_t sid = param->create.service_id;

        if (sid.id.uuid.len == ESP_UUID_LEN_16 && sid.id.uuid.uuid.uuid16 == UUID16_BAS) {
            esp_ble_gatts_start_service(svc_handle);
            esp_gatt_char_prop_t props = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
            esp_bt_uuid_t cuuid = {.len=ESP_UUID_LEN_16, .uuid={.uuid16=UUID16_BATT}};
            esp_attr_value_t init = {.attr_len=1, .attr_max_len=1, .attr_value=(uint8_t[]){100}};
            esp_ble_gatts_add_char(svc_handle, &cuuid, ESP_GATT_PERM_READ, props, &init, NULL);
            batt_handle = svc_handle;

            // Create Tomo Custom Service next
            esp_gatt_srvc_id_t tomo = {.is_primary = true, .id.inst_id = 1};
            tomo.id.uuid.len = ESP_UUID_LEN_128;
            memcpy(tomo.id.uuid.uuid.uuid128, TOMO_SVC_UUID, 16);
            esp_ble_gatts_create_service(gatts_if, &tomo, 6);
        } else {
            // Tomo service
            esp_ble_gatts_start_service(svc_handle);
            esp_bt_uuid_t uuid = {.len=ESP_UUID_LEN_128};
            memcpy(uuid.uuid.uuid128, TOMO_ALERT_UUID, 16);
            esp_gatt_char_prop_t props = ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE;
            esp_attr_value_t dummy = {.attr_len=1, .attr_max_len=20, .attr_value=(uint8_t[]){0}};
            esp_ble_gatts_add_char(svc_handle, &uuid, ESP_GATT_PERM_WRITE, props, &dummy, NULL);
            alert_handle = svc_handle;

            set_adv();
        }
        break;
    }
    case ESP_GATTS_CONNECT_EVT:
        gatts_conn_id = param->connect.conn_id;
        ESP_LOGI(TAG, "connected");
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "disconnected");
        set_adv();
        break;
    default: break;
    }
}

void ble_init(void) {
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    esp_ble_gap_register_callback(gap_cb);
    esp_ble_gatts_register_callback(gatts_cb);
    esp_ble_gatts_app_register(0x55);
}

void ble_set_device_name(const char *name) {
    if (name) {
        strncpy(dev_name, name, sizeof(dev_name)-1);
        dev_name[sizeof(dev_name)-1] = 0;
    }
}

void ble_start_advertising(void) {
    set_adv();
}

void ble_update_battery(uint8_t pct) {
    // In a refined version, write to BAS char value handle; notify if subscribed.
    (void)pct;
}

void ble_update_motion(float ax, float ay, float az) {
    (void)ax; (void)ay; (void)az;
}

void ble_send_alert(void) {
    // For now just log; extend to notify on TOMO_ALERT characteristic
    // with a small payload (e.g., 0x01 = button press)
    ESP_LOGW(TAG, "send alert (notify in next iteration)");
}
