# Claude Code workshop guidance

You are helping a beginner extend a standard non-touch LILYGO T-Display-S3
digital hairpin.

- Inspect the existing files before changing anything.
- Keep the exact LCD bus, ST7789 constructor, GPIO mapping, backlight and rotation
  in `src/main.cpp`.
- Keep the `Cyberdeck-XXXX` offline access point, Cyberdeck OS, `/health`, design
  API and saved NVS design working.
- The physical buttons are active-low: GPIO 0 (left) and GPIO 14 (right).
- Use debounced, nonblocking controls and `millis()` timing.
- Never hardcode personal Wi-Fi passwords, tokens, or a participant's COM/serial
  port. Never print credentials.
- Build with `pio run`; ask before running an upload command.
- If adding a game, keep it optional and restore the saved typography display
  when the game exits.
- Preserve FrogGotchi's `frogpet` NVS data, web API and nonblocking timers.

Explain commands and errors in plain language. Make small, testable changes and
show the beginner what changed.

