#include "brightness_service.h"
#include "bsp/esp_mosaico.h"
#include "nvs.h"
#include "esp_log.h"

static int brightness_percent=40;
static int saved_percent=-1;

int brightness_get_percent(void) { return brightness_percent; }
int brightness_set_percent(int percent)
{
    if (percent<5 || percent>100) return ESP_ERR_INVALID_ARG;
    esp_err_t error=bsp_display_brightness_set(percent);
    if (error==ESP_OK) brightness_percent=percent;
    return error;
}
int brightness_initialize(void)
{
    nvs_handle_t storage;
    uint8_t stored=40;
    esp_err_t error=nvs_open("clock_display",NVS_READONLY,&storage);
    if (error==ESP_OK) {
        error=nvs_get_u8(storage,"brightness",&stored);
        nvs_close(storage);
    }
    if (error==ESP_OK && stored>=5 && stored<=100) saved_percent=stored;
    else if (error!=ESP_OK && error!=ESP_ERR_NVS_NOT_FOUND)
        ESP_LOGW("brightness","Cannot read saved brightness: %s",esp_err_to_name(error));
    return brightness_set_percent(saved_percent>=5 ? saved_percent : 40);
}
int brightness_save(void)
{
    if (saved_percent==brightness_percent) return ESP_OK;
    nvs_handle_t storage;
    esp_err_t error=nvs_open("clock_display",NVS_READWRITE,&storage);
    if (error!=ESP_OK) return error;
    error=nvs_set_u8(storage,"brightness",(uint8_t)brightness_percent);
    if (error==ESP_OK) error=nvs_commit(storage);
    nvs_close(storage);
    if (error==ESP_OK) saved_percent=brightness_percent;
    return error;
}
