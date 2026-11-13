#pragma once
#include "esp_gsp.h"
#include <stdbool.h>
typedef struct app_clock *app_clock_handle_t;
typedef struct {
    int minute;
    bool synchronized;
    esp_gsp_err_t last_error;
} app_clock_snapshot_t;
/* Borrows ui until destroy; call from the UI loop. Output cleared on failure. */
esp_gsp_err_t app_clock_create(esp_gsp_handle_t ui, app_clock_handle_t *out);
/* Read only on the UI loop or after it has stopped. */
esp_gsp_err_t app_clock_read(app_clock_handle_t clock, app_clock_snapshot_t *out);
/* Quiesce timer dispatch first; borrowed ui must remain alive through flush. */
esp_gsp_err_t app_clock_destroy(app_clock_handle_t *clock);
