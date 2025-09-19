#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h" // still needed for headers
#include "host/ble_hs.h"
#include "host/ble_hs_id.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble_pairing.h"

static const char *TAG = "BLE_PAIRING";
static TaskHandle_t nimble_task_handle = NULL;

/* --------- GAP Event Handler --------- */
static int gap_event_cb(struct ble_gap_event *event, void *arg) {
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            ESP_LOGI(TAG, "Connected, handle=%d", event->connect.conn_handle);
        } else {
            ESP_LOGW(TAG, "Connection failed; status=%d", event->connect.status);
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected, reason=%d", event->disconnect.reason);
        break;

    default:
        break;
    }

    return 0;
}

/* --------- Host Reset/Sync --------- */
static void on_reset(int reason) {
    ESP_LOGW(TAG, "NimBLE reset, reason=%d", reason);
}

static void on_sync(void) {
    ESP_LOGI(TAG, "NimBLE host synced");

    uint8_t addr_val[6] = {0};
    int rc = ble_hs_id_infer_auto(0, &addr_val[0]);
    if (rc != 0) {
        ESP_LOGE(TAG, "Error determining address: %d", rc);
    }

    ble_svc_gap_device_name_set("TomoPendant");
    ESP_LOGI(TAG, "Device advertising as TomoPendant");

    struct ble_gap_adv_params adv_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
    };

    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)"TomoPendant";
    fields.name_len = strlen("TomoPendant");
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);
    ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER,
                      &adv_params, gap_event_cb, NULL);
}

/* --------- Host Task --------- */
static void nimble_host_task(void *param) {
    (void)param;
    nimble_port_run();   // This blocks until nimble_port_stop() is called
    nimble_port_deinit();
    vTaskDelete(NULL);
}

/* --------- Public API --------- */
void ble_pairing_init(void) {
    ESP_ERROR_CHECK(esp_nimble_hci_init());
    nimble_port_init();

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb  = on_sync;

    // Launch NimBLE host task manually (ESP-IDF v5.x style)
    xTaskCreate(nimble_host_task, "nimble_host", 4096, NULL, 5, &nimble_task_handle);

    ESP_LOGI(TAG, "BLE pairing initialized");
}

void ble_pairing_deinit(void) {
    if (nimble_task_handle) {
        nimble_port_stop();
        nimble_port_deinit();
        nimble_task_handle = NULL;
    }
    ESP_LOGI(TAG, "BLE pairing deinitialized");
}
