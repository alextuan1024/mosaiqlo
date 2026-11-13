# Third-party notices

The root Apache-2.0 license covers application code unless a file carries another license. It does not replace the licenses of bundled fonts or external dependencies.

| Material | Source and authors | Terms retained here |
| --- | --- | --- |
| `components/app_clock/ui/fonts/gluqlo.ttf` and derived `components/app_clock/digit_masks.h` | Kuźniarski Jacek (2010–2012), Alexander Kovalenko (2014); obtained from [Open Fliqlo](https://github.com/CogitoForge-AI/Open-Fliqlo/tree/main/packages/fliqlo_ui/fonts) | [ISC-style permission notice](components/app_clock/ui/fonts/LICENSE-gluqlo.txt); also embedded in the generated header |
| `DejaVuSans.ttf`, `DejaVuSans-Bold.ttf` | [DejaVu / Bitstream Vera](https://dejavu-fonts.github.io/) | [Bundled license and copyright notices](components/app_clock/ui/fonts/LICENSE) |
| Apache-2.0-marked application scaffolding, display integration and host integration | [ESP-Mosaico hello_world template](https://github.com/esp-mosaico/esp-mosaico-utils/tree/main/mosaico-tools/templates/hello_world), Espressif | Apache-2.0; existing source notices retained; adapted for mosaiqlo |

ESP-IDF, ESP-Mosaico BSP, ESP-Iris, Vibe Mode integration, ESP-GSP and LVGL are external dependencies, not vendored into this repository. Their own licenses and notices continue to apply. Review those notices when distributing compiled firmware.

Fliqlo is the visual inspiration, not a source dependency. No Fliqlo code or proprietary font is distributed. Open Fliqlo is credited as the source of the Gluqlo font; its application code is not included.

## External tooling licensing limitation

The pinned esp-mosaico-utils revision states in [LICENSES.md](https://github.com/esp-mosaico/esp-mosaico-utils/blob/be151ae047014e79a9c7ae0092dd203844499003/LICENSES.md) that migrated Recovery and host-tool sources lack a repository-level license grant. This repository does not relicense those sources or redistribute them. Apache-2.0 source headers on the adapted template files are retained. No compiled firmware is distributed here; resolve upstream licensing before redistributing firmware containing those dependencies.
