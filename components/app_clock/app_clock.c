#include "app_clock.h"
#include "flip_face.h"
#include "app_settings.h"
#define GSP_BUNDLE_ENABLE_RAW_IDS
#include "bundle_gsp.h"
#include <stdlib.h>
#include <time.h>

#define FRAME_INTERVAL_MS 40
#define SINGAPORE_OFFSET_SECONDS (8 * 60 * 60)
#define VALID_TIME_THRESHOLD 1700000000

struct app_clock {
    esp_gsp_handle_t ui;
    void *timer;
    app_settings_t *settings;
    flip_face_t face;
    uint64_t frame_ms;
    esp_gsp_err_t last_error;
};

static uint64_t monotonic_ms(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint64_t)now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

static void remember_error(struct app_clock *clock, esp_gsp_err_t error)
{
    if (error != ESP_GSP_OK) clock->last_error = error;
}

static void draw_face(const esp_gsp_canvas_surface_t *surface, void *context)
{
    struct app_clock *clock = context;
    for (unsigned row = 0; row < surface->height; ++row) {
        uint16_t *pixels = (uint16_t *)((uint8_t *)surface->pixels + row * surface->stride_bytes);
        for (unsigned col = 0; col < surface->width; ++col) {
            uint8_t gray = flip_face_pixel(&clock->face, clock->frame_ms,
                                          surface->x + col, surface->y + row);
            pixels[col] = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
        }
    }
}

static void tick(esp_gsp_handle_t ui, void *context)
{
    struct app_clock *clock = context;
    time_t now = time(NULL);
#ifndef ESP_PLATFORM
    /* A simulator-only accelerated midnight test; device always uses real time. */
    const char *demo = getenv("FLIP_CLOCK_DEMO");
    if (demo && *demo) now = 1794499140 + (monotonic_ms() / 2000 % 4) * 60;
#endif
    if (now < VALID_TIME_THRESHOLD) return;
    time_t singapore = now + SINGAPORE_OFFSET_SECONDS;
    struct tm parts;
    if (!gmtime_r(&singapore, &parts)) return;
    uint64_t frame_ms = monotonic_ms();
    bool first = !clock->face.initialized;
    if (flip_face_update(&clock->face, parts.tm_hour * 60 + parts.tm_min, frame_ms)) {
        clock->frame_ms = frame_ms;
        remember_error(clock, esp_gsp_canvas_invalidate(ui, GSP_FLIP_BIND_FACE));
    }
    if (first) remember_error(clock, gsp_flip_status_set_text(ui, ""));
}

esp_gsp_err_t app_clock_create(esp_gsp_handle_t ui, app_clock_handle_t *out)
{
    if (!out) return ESP_GSP_ERR_INVALID_ARG;
    *out = NULL;
    if (!ui) return ESP_GSP_ERR_INVALID_ARG;
    struct app_clock *clock = calloc(1, sizeof(*clock));
    if (!clock) return ESP_GSP_ERR_NO_MEM;
    clock->ui = ui;
    clock->frame_ms = monotonic_ms();
    esp_gsp_err_t error = esp_gsp_canvas_set_draw_cb(ui, GSP_FLIP_BIND_FACE, draw_face, clock);
    if (error != ESP_GSP_OK) { free(clock); return error; }
    clock->timer = esp_gsp_timer_create(ui, FRAME_INTERVAL_MS, tick, clock);
    if (!clock->timer) {
        esp_gsp_canvas_stop(ui, GSP_FLIP_BIND_FACE);
        esp_gsp_flush(ui, UINT32_MAX);
        free(clock);
        return ESP_GSP_ERR_NO_MEM;
    }
    error = app_settings_create(ui, &clock->settings);
    if (error != ESP_GSP_OK) { app_clock_destroy(&clock); return error; }
    *out = clock;
    return ESP_GSP_OK;
}

esp_gsp_err_t app_clock_read(app_clock_handle_t clock, app_clock_snapshot_t *out)
{
    if (!clock || !out) return ESP_GSP_ERR_INVALID_ARG;
    *out = (app_clock_snapshot_t){clock->face.minute, clock->face.initialized, clock->last_error};
    return ESP_GSP_OK;
}

esp_gsp_err_t app_clock_destroy(app_clock_handle_t *handle)
{
    if (!handle) return ESP_GSP_ERR_INVALID_ARG;
    struct app_clock *clock = *handle;
    if (!clock) return ESP_GSP_OK;
    app_settings_destroy(clock->settings);
    clock->settings = NULL;
    esp_gsp_err_t error;
    if (clock->timer) {
        error = esp_gsp_timer_delete(clock->ui, clock->timer);
        if (error != ESP_GSP_OK) return error;
        clock->timer = NULL;
    }
    error = esp_gsp_canvas_stop(clock->ui, GSP_FLIP_BIND_FACE);
    if (error != ESP_GSP_OK) return error;
    error = esp_gsp_flush(clock->ui, UINT32_MAX);
    if (error != ESP_GSP_OK) return error;
    free(clock);
    *handle = NULL;
    return ESP_GSP_OK;
}
