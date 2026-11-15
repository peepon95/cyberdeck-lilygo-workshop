# Claude Code workshop guidance

You are helping a beginner extend a standard non-touch LILYGO T-Display-S3
Cyber Deck starter.

- Inspect the existing files before changing anything.
- Keep the exact LCD bus, ST7789 constructor, GPIO mapping, backlight and rotation
  in `src/main.cpp`.
- Keep the `LILYGO-SETUP-XXXXXX` access point, `/health`, Wi-Fi setup page,
  connection timeout and saved NVS credentials working.
- The physical buttons are active-low: GPIO 0 (left) and GPIO 14 (right).
- Use debounced, nonblocking controls and `millis()` timing.
- Never hardcode personal Wi-Fi passwords, tokens, or a participant's COM/serial
  port. Never print credentials.
- Build with `pio run`; ask before running an upload command.
- Keep games out of the beginner starter unless the participant explicitly asks
  to add one. Advanced examples live under `examples/advanced-cyberdeck/`.

Explain commands and errors in plain language. Make small, testable changes and
show the beginner what changed.

