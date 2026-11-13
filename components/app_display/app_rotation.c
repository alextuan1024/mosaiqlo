#include "app_display.h"
#include "orientation.h"
#include "bsp/esp_mosaico.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>

static const char *TAG = "clock_rotation";

/* Owned by the task until reboot; the application keeps the UI alive. */
typedef struct {
    esp_gsp_handle_t ui;
    orientation_t orientation;
} rotation_context_t;

static void rotation_task(void *argument)
{
    rotation_context_t *context = argument;
    bool first_sample = true;
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
        float x, y, z;
        if (bsp_imu_get_accel(&x, &y, &z) != ESP_OK) continue;
        if (first_sample) {
            ESP_LOGI(TAG, "Initial acceleration[g]: x=%.3f y=%.3f z=%.3f", x, y, z);
            first_sample = false;
        }
        int previous_candidate = context->orientation.candidate;
        int rotation = orientation_update(&context->orientation, x, y, z, esp_timer_get_time() / 1000);
        if (previous_candidate != context->orientation.candidate)
            ESP_LOGI(TAG, "Orientation sample[g]: x=%.3f y=%.3f z=%.3f candidate=%d",
                     x, y, z, context->orientation.candidate);
        if (rotation == (int)bsp_display_get_rotation()) continue;
        esp_gsp_esp_lcd_pause_t *pause = NULL;
        esp_err_t error = esp_gsp_esp_lcd_pause(context->ui, 1000, &pause);
        if (error != ESP_OK) {
            ESP_LOGW(TAG, "Cannot pause display: %s", esp_err_to_name(error));
            continue;
        }
        error = bsp_display_set_rotation((bsp_display_rotation_t)rotation);
        if (error != ESP_OK) ESP_LOGW(TAG, "Cannot rotate display: %s", esp_err_to_name(error));
        /* Retain the token on failure so rendering can always be resumed. */
        while ((error = esp_gsp_esp_lcd_resume_paused(pause, &context->ui)) != ESP_OK) {
            ESP_LOGE(TAG, "Cannot resume display: %s; retrying", esp_err_to_name(error));
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ESP_LOGI(TAG, "Display rotation=%d", (int)bsp_display_get_rotation());
    }
}

esp_err_t app_display_start_auto_rotation(esp_gsp_handle_t ui)
{
    if (!ui) return ESP_ERR_INVALID_ARG;
    rotation_context_t *context = calloc(1, sizeof(*context));
    if (!context) return ESP_ERR_NO_MEM;
    context->ui = ui;
    context->orientation.candidate = -1;
    context->orientation.rotation = bsp_display_get_rotation();
    esp_err_t error = bsp_imu_init();
    bool initialized = error == ESP_OK;
    if (initialized) error = bsp_imu_start(NULL);
    if (error == ESP_OK && xTaskCreate(rotation_task, "clock_rotation", 4096, context, 3, NULL) != pdPASS)
        error = ESP_ERR_NO_MEM;
    if (error != ESP_OK) {
        if (initialized) (void)bsp_imu_deinit();
        free(context);
    }
    return error;
}
