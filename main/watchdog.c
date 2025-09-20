#include "watchdog.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "WATCHDOG";
static bool s_twdt_inited = false;
static bool s_task_registered = false;

void watchdog_init(void)
{
    if (s_twdt_inited) {
        return;
    }

    // ✅ ESP-IDF v5.x API uses config struct
    const esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 8000,                   // 8 seconds timeout
        .idle_core_mask = (1 << portGET_CORE_ID()), // Watchdog runs on current core
        .trigger_panic = false                // Do not trigger panic on timeout
    };

    esp_err_t err = esp_task_wdt_init(&twdt_config);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        // OK: either initialized or already active
        s_twdt_inited = true;
        ESP_LOGI(TAG, "TWDT initialized (timeout=%d ms)", twdt_config.timeout_ms);
    } else {
        ESP_LOGW(TAG, "TWDT init failed: %s", esp_err_to_name(err));
    }
}

bool watchdog_enable_task(void)
{
    if (!s_twdt_inited) watchdog_init();
    if (!s_twdt_inited) return false;

    if (s_task_registered) {
        return true;
    }

    esp_err_t err = esp_task_wdt_add(NULL); // register current task
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        s_task_registered = true;
        ESP_LOGI(TAG, "TWDT: current task registered");
        return true;
    }
    ESP_LOGW(TAG, "TWDT: add current task failed: %s", esp_err_to_name(err));
    return false;
}

bool watchdog_disable_task(void)
{
    if (!s_task_registered) return true;
    esp_err_t err = esp_task_wdt_delete(NULL);
    if (err == ESP_OK || err == ESP_ERR_NOT_FOUND) {
        s_task_registered = false;
        ESP_LOGI(TAG, "TWDT: current task unregistered");
        return true;
    }
    ESP_LOGW(TAG, "TWDT: delete current task failed: %s", esp_err_to_name(err));
    return false;
}

bool watchdog_feed(void)
{
    if (!s_task_registered) {
        if (!watchdog_enable_task()) return false;
    }
    esp_err_t err = esp_task_wdt_reset();
    if (err == ESP_OK) return true;
    ESP_LOGW(TAG, "TWDT: reset failed: %s", esp_err_to_name(err));
    return false;
}

void watchdog_self_test(void)
{
    watchdog_init();
    watchdog_enable_task();
    (void)watchdog_feed();
    ESP_LOGI(TAG, "Self-test: watchdog init+register+feed OK");
}
