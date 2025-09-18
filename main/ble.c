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
#include "host/ble_store.h"             // for ble_store_util_status_rr
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "ble.h"

static const char *TAG = "BLE_NIMBLE";

/* ------- State ------- */
static char s_dev_name[32] = "TomoPendant";
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint8_t  s_own_addr_type = BLE_OWN_ADDR_PUBLIC;

static uint8_t s_batt_pct = 100;

static volatile bool s_sub_batt   = false;
static volatile bool s_sub_alert  = false;
static volatile bool s_sub_motion = false;

static uint16_t s_batt_val_handle   = 0;
static uint16_t s_alert_val_handle  = 0;
static uint16_t s_motion_val_handle = 0;

/* UUIDs */
static const ble_uuid128_t TOMO_SVC_UUID =
    BLE_UUID128_INIT(0x44,0x33,0x22,0x11,0x66,0x55,0x22,0x9a,0xb1,0x4a,0x0d,0x5c,0x10,0x21,0x33,0x9a);
static const ble_uuid128_t TOMO_ALERT_UUID =
    BLE_UUID128_INIT(0x45,0x33,0x22,0x11,0x66,0x55,0x22,0x9a,0xb1,0x4a,0x0d,0x5c,0x10,0x21,0x33,0x9a);
static const ble_uuid128_t TOMO_MOTION_UUID =
    BLE_UUID128_INIT(0x46,0x33,0x22,0x11,0x66,0x55,0x22,0x9a,0xb1,0x4a,0x0d,0x5c,0x10,0x21,0x33,0x9a);

/* Forward decls */
static int gatt_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gap_event_cb(struct ble_gap_event *event, void *arg);
static void start_advertising(void);

/* ------- GATT Services ------- */
static const struct ble_gatt_chr_def gatt_bas_chrs[] = {
    {
        .uuid = BLE_UUID16_DECLARE(0x2A19),
        .access_cb = gatt_chr_access_cb,
        .val_handle = &s_batt_val_handle,
        .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
    },
    { 0 }
};

static const struct ble_gatt_svc_def gatt_battery_svc[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0x180F),
        .characteristics = gatt_bas_chrs,
    },
    { 0 }
};

static const struct ble_gatt_chr_def gatt_tomo_chrs[] = {
    {
        .uuid = &TOMO_ALERT_UUID.u,
        .access_cb = gatt_chr_access_cb,
        .val_handle = &s_alert_val_handle,
        .flags = BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_WRITE,
    },
    {
        .uuid = &TOMO_MOTION_UUID.u,
        .access_cb = gatt_chr_access_cb,
        .val_handle = &s_motion_val_handle,
        .flags = BLE_GATT_CHR_F_NOTIFY,
    },
    { 0 }
};

static const struct ble_gatt_svc_def gatt_tomo_svc[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &TOMO_SVC_UUID.u,
        .characteristics = gatt_tomo_chrs,
    },
    { 0 }
};

static const struct ble_gatt_svc_def *gatt_svcs[] = {
    gatt_battery_svc,
    gatt_tomo_svc,
    NULL
};

/* ------- GATT Access ------- */
static int gatt_chr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle;
    (void)arg;
    const ble_uuid_t *uuid = ctxt->chr->uuid;

    if (ble_uuid_u16(uuid) == 0x2A19) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
            uint8_t val = s_batt_pct;
            return os_mbuf_append(ctxt->om, &val, sizeof(val)) == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        return BLE_ATT_ERR_UNLIKELY;
    }

    if (ble_uuid_cmp(uuid, &TOMO_ALERT_UUID.u) == 0) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            uint8_t buf[20];
            int len = ble_hs_mbuf_to_flat(ctxt->om, buf, sizeof(buf), NULL);
            ESP_LOGI(TAG, "Alert WRITE, %d bytes", len);
            return 0;
        }
        return BLE_ATT_ERR_UNLIKELY;
    }

    return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
}

/* ------- GAP Events ------- */
static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            s_conn_handle = event->connect.conn_handle;
            ESP_LOGI(TAG, "Connected; handle=%u", s_conn_handle);
        } else {
            ESP_LOGW(TAG, "Connect failed; status=%d", event->connect.status);
            start_advertising();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "Disconnected; reason=%d", event->disconnect.reason);
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_sub_batt = s_sub_alert = s_sub_motion = false;
        start_advertising();
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(TAG, "Advertising complete; reason=%d", event->adv_complete.reason);
        start_advertising();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.attr_handle == s_batt_val_handle) {
            s_sub_batt = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == s_alert_val_handle) {
            s_sub_alert = event->subscribe.cur_notify;
        } else if (event->subscribe.attr_handle == s_motion_val_handle) {
            s_sub_motion = event->subscribe.cur_notify;
        }
        return 0;

    default:
        return 0;
    }
}

/* ------- Advertising ------- */
static void start_advertising(void)
{
    struct ble_gap_adv_params advp = {0};
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;

    struct ble_hs_adv_fields fields = {0};
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)s_dev_name;
    fields.name_len = (uint8_t)strlen(s_dev_name);
    fields.name_is_complete = 1;
    fields.num_uuids128 = 1;
    fields.uuids128 = &TOMO_SVC_UUID;
    fields.uuids128_is_complete = 1;
    int rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "adv_set_fields rc=%d", rc);
        return;
    }

    struct ble_hs_adv_fields rsp = {0};
    static ble_uuid16_t bas = { .u = { .type = BLE_UUID_TYPE_16 }, .value = 0x180F };
    rsp.uuids16 = &bas;
    rsp.num_uuids16 = 1;
    rsp.uuids16_is_complete = 1;
    ble_gap_adv_rsp_set_fields(&rsp);

    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &advp, gap_event_cb, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "adv_start rc=%d", rc);
    } else {
        ESP_LOGI(TAG, "Advertising as '%s' (own_addr_type=%u)", s_dev_name, s_own_addr_type);
    }
}

/* ------- Host sync/reset ------- */
static void on_sync(void)
{
    int rc = ble_hs_id_infer_auto(0, &s_own_addr_type);
    if (rc != 0) {
        ESP_LOGE(TAG, "infer_auto rc=%d", rc);
        s_own_addr_type = BLE_OWN_ADDR_RANDOM;
    }

    uint8_t addr_val[6] = {0};
    ble_hs_id_copy_addr(s_own_addr_type, addr_val, NULL);
    ESP_LOGI(TAG, "BLE addr: %02X:%02X:%02X:%02X:%02X:%02X",
             addr_val[5], addr_val[4], addr_val[3], addr_val[2], addr_val[1], addr_val[0]);

    rc = ble_gatts_count_cfg(gatt_svcs);
    if (!rc) rc = ble_gatts_add_svcs(gatt_svcs);
    if (rc != 0) {
        ESP_LOGE(TAG, "add_svcs rc=%d", rc);
        return;
    }

    ble_svc_gap_device_name_set(s_dev_name);
    start_advertising();
}

static void on_reset(int reason) { ESP_LOGW(TAG, "NimBLE reset; reason=%d", reason); }

static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/* ------- Public API ------- */
void ble_init(void)
{
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb  = on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    nimble_port_freertos_init(ble_host_task);
}

void ble_set_device_name(const char *name)
{
    if (name && name[0]) {
        size_t n = strlen(name);
        if (n >= sizeof(s_dev_name)) n = sizeof(s_dev_name) - 1;
        memcpy(s_dev_name, name, n);
        s_dev_name[n] = '\0';
        if (ble_hs_synced()) ble_svc_gap_device_name_set(s_dev_name);
    }
}

void ble_start_advertising(void)
{
    if (ble_hs_synced()) start_advertising();
}

void ble_update_battery(uint8_t percent)
{
    if (percent > 100) percent = 100;
    s_batt_pct = percent;

    if (s_conn_handle != BLE_HS_CONN_HANDLE_NONE && s_sub_batt && s_batt_val_handle != 0) {
        struct os_mbuf *om = ble_hs_mbuf_from_flat(&s_batt_pct, sizeof(s_batt_pct));
        ble_gatts_notify_custom(s_conn_handle, s_batt_val_handle, om);
    }
}

static inline int16_t clamp_i16(int v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

void ble_update_motion(float ax, float ay, float az)
{
    if (!(s_conn_handle != BLE_HS_CONN_HANDLE_NONE && s_sub_motion && s_motion_val_handle != 0))
        return;

    int16_t payload[3] = {
        clamp_i16((int)(ax * 1000.0f)),
        clamp_i16((int)(ay * 1000.0f)),
        clamp_i16((int)(az * 1000.0f))
    };

    struct os_mbuf *om = ble_hs_mbuf_from_flat(payload, sizeof(payload));
    ble_gatts_notify_custom(s_conn_handle, s_motion_val_handle, om);
}

void ble_send_alert_code(uint8_t code)
{
    if (!(s_conn_handle != BLE_HS_CONN_HANDLE_NONE && s_sub_alert && s_alert_val_handle != 0))
        return;

    struct os_mbuf *om = ble_hs_mbuf_from_flat(&code, sizeof(code));
    ble_gatts_notify_custom(s_conn_handle, s_alert_val_handle, om);
}
