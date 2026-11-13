# mosaiqlo

A Fliqlo-inspired flip clock for ESP-Mosaico (ESP32-S31), with Gluqlo digits and a 480 × 480 AMOLED interface. Independent implementation; not affiliated with or endorsed by Fliqlo.

## Features

- 24-hour Singapore time (UTC+8), with animated minute transitions.
- Automatic rotation in all four orientations.
- Long-press the clock to open Settings.
- Wi-Fi scanning, saved networks, open networks, and an on-screen password keyboard.
- Brightness control from 5–100%, saved across restarts.
- SNTP synchronization using `pool.ntp.org`; keeps time during Wi-Fi outages after synchronization.

A full power-off loses the current time; internet access is needed to synchronize again. Sleep mode is not implemented. Wi-Fi credentials are stored in device NVS; existing Vibe Mode credentials can be reused.

## Build and run

This is an application repository, intended to live under `projects/mosaiqlo` in the [ESP-Mosaico workspace](https://github.com/esp-mosaico/esp-mosaico-vibe). It is not a standalone ESP-IDF checkout.

```sh
git clone https://github.com/esp-mosaico/esp-mosaico-vibe.git mosaico-workspace
cd mosaico-workspace
git checkout d6aa4a8d3277b0c7ad3d38e54c403f2388814d1e
git submodule update --init --recursive
git clone https://github.com/alextuan1024/mosaiqlo.git projects/mosaiqlo
```

Use ESP-IDF at **`7b9cc1ac79f865983f59bb8ff3ff43eb74ff1dbe`**, with ESP32-S31 support and its installed toolchain. Follow the parent workspace's environment setup instructions, then source your SDK's `export.sh`.

From the workspace root:

```sh
python3 mosaico.py project sim --project projects/mosaiqlo --interactive
python3 mosaico.py iris list --project projects/mosaiqlo --details --json
python3 mosaico.py iris system-update --project projects/mosaiqlo
```

Follow the parent workspace's device selection and installation instructions. Preserve the retained Vibe Mode firmware and partition layout; do not erase the whole flash. Installation is a separate step from cloning or building.

The source was developed against these workspace dependencies:

| Dependency | Revision |
| --- | --- |
| esp-mosaico-utils | `be151ae047014e79a9c7ae0092dd203844499003` |
| esp-mosaico-bsp | `5f88b91e1be055d1d3674364123b2779a1758377` |
| ESP-GSP | `1.5.1` |
| LVGL | `9.5.0` |

## Tests

From this repository, run the portable C tests:

```sh
cc -Wall -Wextra -Werror -I components/app_clock/include tests/test_face.c components/app_clock/flip_face.c -o /tmp/mosaiqlo-face
/tmp/mosaiqlo-face
cc -Wall -Wextra -Werror -I components/app_display/private tests/test_orientation.c components/app_display/orientation.c -o /tmp/mosaiqlo-orientation
/tmp/mosaiqlo-orientation
cc -Wall -Wextra -Werror -I components/app_time/private -I components/app_time/include tests/test_wifi_profiles.c components/app_time/wifi_profiles.c -o /tmp/mosaiqlo-wifi
/tmp/mosaiqlo-wifi
```

After building the simulator, `tests/test_settings_sim.py` and `tests/test_brightness_sim.py` exercise its UI through the GSP bridge. They require Pillow and the ESP-GSP simulator runtime; set `MOSAICO_SIM` if the executable is not at `~/.cache/esp-mosaico/sim/1.5.1/sim`. Run them sequentially. Set `FLIP_CLOCK_DEMO=1` when running the simulator for accelerated midnight transitions; the device uses real time.

## Known limitation

The installed clock has intermittently failed to enumerate over USB after restart, while Vibe Mode remains reachable. The cause is unresolved. Enter Vibe Mode by holding AI during power-on when necessary. This repository does not claim a fix for that issue.

## Credits and license

Application code is licensed under [Apache-2.0](LICENSE), except separately licensed third-party assets. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and [NOTICE](NOTICE), including the pinned external tooling’s incomplete licensing. No firmware binaries are distributed here.

- [Fliqlo](https://fliqlo.com/): visual inspiration. No Fliqlo source code or proprietary font is included.
- **Gluqlo**, by Kuźniarski Jacek and Alexander Kovalenko: digit font, obtained through [Open Fliqlo](https://github.com/CogitoForge-AI/Open-Fliqlo/tree/main/packages/fliqlo_ui/fonts). Its permission notice is retained, including on derived digit masks.
- [DejaVu fonts](https://dejavu-fonts.github.io/): settings UI fonts, with their bundled license retained.
- [Espressif / ESP-Mosaico](https://github.com/esp-mosaico): hardware, BSP, application template, ESP-Iris, Vibe Mode, and development tooling; [ESP-IDF](https://github.com/espressif/esp-idf), ESP-GSP, and [LVGL](https://github.com/lvgl/lvgl) provide the firmware and UI foundations.
