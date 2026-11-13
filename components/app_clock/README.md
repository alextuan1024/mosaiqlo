# app_clock

Portable 24-hour flip-clock controller and grayscale face renderer, shared by ESP-IDF and the native simulator. Uses GSP 1.5.1's RGB565 direct-draw Canvas. The scene is 480 x 480; two 216 x 224 cards show hours and minutes. A 600 ms two-stage flip compresses the outgoing top flap and expands the incoming bottom flap. Only changed cards animate. Long-press anywhere on the clock to open Settings.

`app_clock_create` borrows the GSP handle and owns a 40 ms timer and private model. Time comes from the platform's wall clock, displayed at UTC+8. Invalid time displays dashes and the synchronization notice. The callback only repaints on a minute change or during a flip; the idle timer does not repaint continuously. Setters and rendering run on the UI loop. Snapshots must be read on that loop or after dispatch stops. No ISR or cross-task calls are supported.

Quiesce callbacks before `app_clock_destroy`; it deletes the timer, stops Canvas, flushes borrowed state, then frees the controller. Failures retain ownership for retry. Creation clears the output pointer on error. Runtime GSP errors are retained in the diagnostic snapshot.

`flip_face.c` has no SDK dependency. Its native tests cover startup, midnight rollover, animation completion, invalid time and rendering. Digit masks were rasterized from the bundled Gluqlo font (license in ui/fonts/LICENSE-gluqlo.txt), scaled to 80 x 152 per glyph. Font source: https://github.com/CogitoForge-AI/Open-Fliqlo/tree/main/packages/fliqlo_ui/fonts. No Fliqlo code or assets are included.

The desktop-only `FLIP_CLOCK_DEMO=1` setting advances minutes every two seconds through midnight for animation testing. Firmware ignores it.

`app_settings.c` owns the Settings → Wi-Fi → Password navigation, native keyboard key events, masked ASCII password buffer (64 bytes maximum), and a dynamic scrollable AP list. The built-in long gesture opens Settings; Back returns one level. The singleton settings context stays alive until UI destruction because GSP retains its list callback. Its timer and event callback stop during clock destruction, and password bytes are explicitly cleared on leaving entry. The host substitutes only Wi-Fi service data; UI and input handling are shared with hardware.

Run `tests/test_settings_sim.py` after building the native backend to exercise long-press, saved/open/password connections, failed-password retry, masking, and Back navigation. Simulator screenshots and sanitized service results are saved under `build-sim/settings-test`.
