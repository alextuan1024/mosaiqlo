// SPDX-License-Identifier: Apache-2.0
// Modified for mosaiqlo: flip-clock application integration.
#include <stdio.h>
#include "app_clock.h"
#include "gsp_sim_bridge.h"

/* The bridge ABI has no caller context. Keep only its owned opaque handle here. */
static struct {
    app_clock_handle_t controller;
} backend;

esp_gsp_err_t gsp_bridge_app_init(esp_gsp_handle_t ui)
{
    return app_clock_create(ui, &backend.controller);
}

void gsp_bridge_app_deinit(esp_gsp_handle_t ui)
{
    (void)ui;
    app_clock_snapshot_t snapshot;
    if (app_clock_read(backend.controller, &snapshot) == ESP_GSP_OK) {
        fprintf(stderr, "clock_backend: minute=%u, last_error=%d\n",
                (unsigned)snapshot.minute, (int)snapshot.last_error);
    }
    /* The bridge calls deinit after poll exits; no callbacks can use the state. */
    const esp_gsp_err_t err = app_clock_destroy(&backend.controller);
    if (err != ESP_GSP_OK) {
        fprintf(stderr, "clock_backend: cleanup failed (%d)\n", err);
    }
}
