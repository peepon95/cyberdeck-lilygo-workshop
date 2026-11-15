# Codex workshop guidance

This is a beginner workshop project for a standard non-touch LILYGO T-Display-S3.

Before editing:

1. Read `README.md`, `platformio.ini`, `src/main.cpp`, and `src/studio_page.h`.
2. Preserve the verified `Arduino_ESP32PAR8Q` bus, `Arduino_ST7789` constructor,
   LCD power GPIO 15, backlight GPIO 38, and `display->setRotation(1)`.
3. Preserve the protected `Cyberdeck-XXXX` access point, Cyberdeck OS routes,
   `/health`, design API, and NVS design persistence unless explicitly asked.
4. Use GPIO 0 for the left button and GPIO 14 for the right button. They are
   active-low; use debouncing and nonblocking input handling.
5. Do not add secrets, personal Wi-Fi credentials, cloud dependencies, or local
   serial-port paths to source control.
6. Prefer `millis()` timing and bounded arrays; do not block HTTP servicing.
7. Run `pio run` and fix errors before offering to upload.
8. Always ask the participant before flashing hardware. Detect the current port
   with `pio device list`; never assume the example port is correct.
9. Preserve FrogGotchi's `frogpet` NVS namespace and nonblocking care/timer loop.

When building a game, keep it optional and make it possible to return to the
Cyberdeck OS launcher and saved Display Studio design. Prioritise one reliable playable loop over elaborate
graphics.

