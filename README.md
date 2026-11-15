# My First LILYGO — Cyber Deck Workshop Starter

Beginner starter firmware for the standard, non-touch **LILYGO T-Display-S3**
(ESP32-S3, 1.9-inch 170×320 ST7789 LCD). The display runs in 320×170 landscape
mode through the board's verified 8-bit parallel interface.

## What participants will see

Public workshop guide: <https://cyberdeck-build-together.pages.dev>

After the first successful upload:

1. `HELLO WORLD!` and `MY FIRST LILYGO` appear for three seconds.
2. The screen changes to **Wi-Fi Setup**.
3. The board creates `LILYGO-SETUP-XXXXXX` with password `cyberdeck`.
4. Open <http://192.168.4.1> and enter a phone hotspot or Wi-Fi network.
5. The board displays **Wi-Fi Connected** and its assigned IP address.

Credentials are saved in ESP32 Preferences/NVS so the board can reconnect after
power cycling. The protected setup access point remains available if a connection
fails. The password is never printed to Serial logs.

## Build

Install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html),
open a terminal in this folder, and run:

```sh
pio run
```

Find the connected port with `pio device list`, then upload after replacing the
example with the detected port:

```sh
# macOS example
pio run -t upload --upload-port /dev/cu.usbmodemXXXX

# Windows example
pio run -t upload --upload-port COMX
```

## Verified hardware configuration

- LCD power: GPIO 15
- Backlight: GPIO 38
- ST7789 chip-select: GPIO 5
- 8-bit bus: DC 7, WR 6, D0–D7 8, 9, 39, 40, 41, 42, 45, 46
- Panel: 170×320 with 35-pixel horizontal offsets, rotated to landscape
- Left button: GPIO 0, active-low
- Right button: GPIO 14, active-low

## Repository layout

```text
platformio.ini                 beginner starter configuration
src/main.cpp                   Hello World and Wi-Fi setup firmware
workshop-guide/                step-by-step Mac/Windows tutorial website
examples/advanced-cyberdeck/   FrogGotchi, Rocket Rush and Display Studio
AGENTS.md                      instructions for Codex
CLAUDE.md                      instructions for Claude Code
```

The games are intentionally excluded from the root starter build. Participants
begin with display and Wi-Fi setup, then create their own projects. The previous
advanced firmware remains available as a separate PlatformIO project under
`examples/advanced-cyberdeck/`.

## Upload recovery

If upload cannot connect, hold **BOOT**, briefly press and release **RST**, release
**BOOT**, detect the port again, and retry. Press **RST** after upload if the new
firmware does not start automatically.
