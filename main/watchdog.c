#include "watchdog.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_task_wdt.h"

static const char *TAG = "WATCHDOG";
static bool s_twdt_inited = false;
static bool s_task_registered = false;

void watchdog_init(void)
{
    if (s_twdt_inited) {
        return;
    }

    // Timeout chosen to be generous for CI / first hardware bring-up.
    // No panic on timeout in this baseline.
    const int timeout_seconds = 8;

    esp_err_t err = esp_task_wdt_init(timeout_seconds, /*panic*/ false);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        // ESP_ERR_INVALID_STATE means it was already initialized – acceptable.
        s_twdt_inited = true;
        ESP_LOGI(TAG, "TWDT initialized (timeout=%ds)", timeout_seconds);
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

    esp_err_t err = esp_task_wdt_add(NULL); // current task
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        // INVALID_STATE -> already added
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
        // Attempt to self-register to be resilient in early bring-up.
        if (!watchdog_enable_task()) return false;
    }
    esp_err_t err = esp_task_wdt_reset();
    if (err == ESP_OK) return true;
    ESP_LOGW(TAG, "TWDT: reset failed: %s", esp_err_to_name(err));
    return false;
}

void watchdog_self_test(void)
{
    // Non-intrusive: verifies API path without forcing resets in CI.
    watchdog_init();
    watchdog_enable_task();
    (void)watchdog_feed();
    ESP_LOGI(TAG, "Self-test: TWDT init+register+feed OK (non-intrusive)");
}
