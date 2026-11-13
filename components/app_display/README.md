# app_display

Adapts the BSP panel and optional touch to GSP's display configuration. It owns
application display policy (RGB565, byte order, rotation, TE and render alignment),
not UI behavior.
The public dependency is ESP-GSP; BSP, display-present, LCD and logging are private
implementation dependencies. There is no dependency on `app_clock` or `main`.

```c
esp_gsp_esp_lcd_config_t display;
esp_err_t err = app_display_init(&display);
/* On ESP_OK, pass &display to esp_gsp_esp_lcd_start with the bundle config. */
```

The CO5300 QSPI panel requires both the refresh window's X origin and width to
be multiples of 4 pixels. `app_display_init()` explicitly sets GSP's
`render_alignment.x_pixels = 4` and `render_alignment.width_pixels = 4` on the
device. Keep both values: GSP defaults to no expansion, and the BSP's area rounder
only handles LVGL. Existing generated applications must apply the same change
to their device display configuration; updating the template does not patch them.

After changing display initialization, verify partial refresh on the physical
panel: repeated counter updates, the celebration animation, and scrolling in
applications that provide it. Check for shifted pixels, corruption and stale
regions. Host simulation and Iris screenshots cannot establish panel alignment;
Iris captures the pixel buffer before the panel transfer.

Call once from boot, before GSP starts; do not call from ISR or concurrently.
Configuration is copied to caller storage only on success. Hardware handles are
borrowed from the singleton BSP and remain alive until reboot. Optional touch
failure yields `ESP_OK` with `display.touch == NULL`; display failure propagates.
The caller owns configuration storage, not hardware handles; do not free those
handles or reinitialize the BSP while GSP uses them. Resolution, pixel clock (Hz),
rotation and TE wiring come from BSP settings. NULL output returns
`ESP_ERR_INVALID_ARG`; missing panel/io returns `ESP_ERR_INVALID_STATE`; other
display initialization errors propagate from the BSP.
Hardware initialization may block using driver timeouts; there is no additional
application deadline. Partial hardware initialization is not rolled back. Keep
Recovery running if UI startup cannot proceed.

Private helpers remain static in the source file. If private headers become
necessary, put them under `private/` and declare only `PRIV_INCLUDE_DIRS`.

`app_display_start_auto_rotation(ui)` is called once after the clock controller
starts. It owns the BMI270 and a boot-lifetime task (4096-byte stack), polls at
100 ms, and accepts a direction after 400 ms of consistent readings. Flat,
diagonal, non-finite, and high/low acceleration readings cancel a pending change;
sample gaps over 250 ms restart the settling interval. Flat placement retains
the last rotation. Sensor startup failure leaves the clock usable without rotation.

Before changing the panel orientation, the task pauses GSP and drains presentation
fences. It always resumes GSP, forcing a full redraw. The BSP rotates touch
coordinates with the panel. Do not destroy the borrowed UI or access the BMI270
from another controller while this service runs; its lifetime ends at reboot.

The portable direction/debounce model is in `orientation.c`; run its assertions
with `tests/test_orientation.c` and the component's `private` include directory.
Actual sensor mounting and hardware scan rotation require physical validation;
Iris screenshots show the logical image before panel rotation.

Mounting check on device `4553502d4952495301001c2904d0ae22`: upright
and readable at rotation 0 measured acceleration (0.055, 0.992, -0.032) g.
The orientation model uses sensor X/Y directly.

Brightness uses the CO5300 hardware register via the BSP, with a 5–100% slider.
The default is 40%. The application stores the last selected value in NVS
namespace `clock_display`, key `brightness`; missing/invalid values use the
default. Initialization follows panel creation, and runtime service calls belong
to the UI task. A successful apply updates the current value; slider release or
Back commits it once, avoiding flash writes on every drag event. Save errors
remain visible and can be retried. Wi-Fi credentials are untouched.
