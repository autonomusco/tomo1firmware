#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "nvs_flash.h"

/* NimBLE (ESP-IDF) */
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "host/ble_hs_id.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble.h"

static const char *TAG = "BLE_NIMBLE";

/* ------- App-level state ------- */
static char s_dev_name[32] = "TomoPendant";
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;

/* Battery value cache (0..100). Updated by app_main. */
static uint8_t s_batt_pct = 100;

/* Subscription flags (client CCCD) */
static volatile bool s_sub_batt = false;
static volatile bool s_sub_alert = false;

/* Handles to characteristic value attributes (set by GATT def) */
static uint16_t s_batt_val_handle = 0;
static uint16_t s_alert_val_handle = 0;

/* Custom 128-bit UUIDs */
static const ble_uuid128_t TOMO_SVC_UUID =
    BLE_UUID128_INIT(0x44,0x33,0x22,0x11,0x66,0x55,0x22,0x9a,0xb1,0x4a,0x0d,0x5c,0x10,0x21,0x33,0x9a);
static const ble_uuid128_t TOMO_ALERT_UUID =
    BLE_UUID128_INIT(0x45,0x33,0x22,0x11,0x66,0x55,0x22,0x9a,0xb1,0x4a,0x0d,0x5c,0x10,0x21,0x33,0x9a);

/* Forward decls */
static int gatt_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gap_event_cb(struct ble_gap_event *event, void *arg);
static void start_advertising(void);

/* ------- GATT: Services & Characteristics ------- */
/* Battery Service (0x180F) with Battery Level (0x2A19*
