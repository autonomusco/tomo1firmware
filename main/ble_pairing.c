#include "ble_pairing.h"
#include "esp_log.h"
#include "esp_err.h"

// NimBLE includes
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "BLE_PAIRING";

// Example GATT services placeholder
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {0}, // terminator
};

static void ble_on_sync(void) {
    ESP_LOGI(TAG, "BLE stack synced, ready for connections");
}

static void ble_hs_task(void *param) {
    ESP_LOGI(TAG, "BLE host task started");
    nimble_port_run();
    nimble_port_freertos_deinit();
}

bool ble_pairing_init(void) {
    ESP_LOGI(TAG, "Initializing BLE pairing…");

    // Initialize NimBLE stack
    int rc = nimble_port_init();
    if (rc != 0) {
        ESP_LOGE(TAG, "Failed to init NimBLE port, rc=%d", rc);
        return false;
    }

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