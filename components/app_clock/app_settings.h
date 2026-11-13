#pragma once
#include "esp_gsp.h"
typedef struct app_settings app_settings_t;
esp_gsp_err_t app_settings_create(esp_gsp_handle_t ui, app_settings_t **out);
/* Called after UI polling stops. */
void app_settings_destroy(app_settings_t *settings);
