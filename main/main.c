// SPDX-License-Identifier: Apache-2.0
// Modified for mosaiqlo: flip-clock application integration.
#include "app_display.h"
#include "app_clock.h"
#include "app_time.h"
#include "esp_iris.h"
#include "esp_log.h"
#include "iris_ota_support.h"
#include "iris_screen_mirror.h"
#include "nvs_flash.h"
#include "ui_bundle.h"

static const char *TAG = "mosaiqlo";

/* The composition root retains owned handles, never controller internals. */
static struct {
    esp_gsp_handle_t ui;
    app_clock_handle_t controller;
} application;

void app_main(void)
{
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_iris_boot_probe());
    ESP_ERROR_CHECK(nvs_flash_init());
    /* Recovery must remain reachable even when the UI bundle cannot be loaded. */
    iris_ota_support_start();
    const esp_err_t time_err = app_time_start();
    if (time_err != ESP_OK) ESP_LOGW(TAG, "Wi-Fi initialization failed: %s; clock remains available", esp_err_to_name(time_err));

    esp_gsp_config_t config;
    const esp_err_t err = ui_bundle_open(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GSP UI unavailable (0x%x); Recovery RPC remains active", err);
        return;
    }
    esp_gsp_esp_lcd_config_t display;
    const esp_err_t display_err = app_display_init(&display);
    if (display_err != ESP_OK) {
        ESP_LOGE(TAG, "Display unavailable (0x%x); Recovery RPC remains active", display_err);
        return;
    }
    ESP_ERROR_CHECK(iris_screen_mirror_init());
    ESP_ERROR_CHECK(esp_gsp_esp_lcd_start(&config, &display, &application.ui));
    ESP_ERROR_CHECK(iris_screen_mirror_attach(application.ui));
    ESP_ERROR_CHECK(app_clock_create(application.ui, &application.controller));
    const esp_err_t rotation_error = app_display_start_auto_rotation(application.ui);
    if (rotation_error != ESP_OK)
        ESP_LOGW(TAG, "Auto rotation unavailable: %s", esp_err_to_name(rotation_error));
    /* GSP retains the callback context. Device UI/controller live until reboot;
     * teardown requires quiescing GSP before app_clock_destroy(), then closing UI. */
    ESP_LOGI(TAG, "Flip clock ready");
}
