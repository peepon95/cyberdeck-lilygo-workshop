# Cyber Deck Workshop Starter

Beginner-friendly firmware for the standard, non-touch **LILYGO T-Display-S3**
(ESP32-S3, 1.9-inch 170×320 ST7789 display). The screen runs in 320×170
landscape mode through the board's verified 8-bit parallel LCD interface.

## Step-by-step workshop guide

The short Mac/Windows guide is stored in [`workshop-guide/`](workshop-guide/).
Open `workshop-guide/index.html` locally, or serve that folder with:

```sh
python3 -m http.server 4173 --directory workshop-guide
```

## What this starter does

- Creates a protected local Wi-Fi network named `Cyberdeck-XXXX`.
- Uses the workshop password `cyberdeck` and shows `192.168.4.1` on screen.
- Hosts Cyberdeck OS and Display Studio directly on the ESP32.
- Lets you edit text, fonts, colours, size, banner/outline/marquee modes.
- Applies changes immediately and saves the design in ESP32 NVS.
- Keeps `GET /health` and `GET/POST /api/design` available.

Cyberdeck OS includes Rocket Rush as a working example and reserves expandable
launcher slots for participant-built projects. FrogGotchi is integrated as the
first virtual-pet and desk-companion experience.

## FrogGotchi

FrogGotchi combines persistent virtual-pet care with a desk focus companion:

- Fullness, energy, happiness, health and cleanliness stats
- Egg, hatchling, young, grown and evolved life stages
- Feed, affection, cleaning, sleeping and focus actions
- Companion Mode (safe) and optional Classic Mode (neglect can be fatal)
- Pomodoro focus sessions, five-minute breaks, quotes, coins and rewards
- Twelve embedded frog expressions selected from the supplied pixel artwork
- State stored separately in the `frogpet` NVS namespace
- Browser clock synchronisation to account for elapsed time after power loss
- Three persistent 320×170 pixel-art homes: Cozy Room, Study Desk and Frog Pond
- Mobile-first My Frog, Focus, Backgrounds, Rewards, Stats and Settings screens

Open FrogGotchi at `http://192.168.4.1`; Cyberdeck OS remains available at `/os`.
On the physical screen, LEFT cycles actions, RIGHT performs the selected action,
and holding both buttons for 1.8 seconds returns to the launcher.

## 1. Install PlatformIO Core

Follow the current official instructions at
<https://docs.platformio.org/en/latest/core/installation/index.html>, then check:

```sh
pio --version
```

## 2. Build

Open a terminal in this folder—the folder containing `platformio.ini`—and run:

```sh
pio run
```

PlatformIO downloads the pinned toolchain and libraries on the first build.

## 3. Find the USB serial port

macOS:

```sh
pio device list
```

Look for a path such as `/dev/cu.usbmodem1101`.

Windows PowerShell:

```powershell
pio device list
```

Look for a port such as `COM5`.

## 4. Upload

Replace the example port with the port shown on your laptop.

macOS:

```sh
pio run --target upload --upload-port /dev/cu.usbmodem1101
```

Windows PowerShell:

```powershell
pio run --target upload --upload-port COM5
```

After uploading, the board normally restarts itself. If the screen does not
change, briefly press **RST** once.

## 5. Open Cyberdeck OS

1. Read the Wi-Fi name and password from the LILYGO screen.
2. Join `Cyberdeck-XXXX` using the password `cyberdeck`.
3. If the laptop says “No internet,” stay connected; that is expected.
4. Open <http://192.168.4.1>.
5. Choose Display Studio, Rocket Rush, or the physical on-screen menu.

The hairpin works offline after flashing. You only need internet when installing
tools, downloading dependencies, or asking an AI assistant for help.

## Upload recovery

If upload cannot connect:

1. Hold the board's **BOOT** button.
2. Briefly press and release **RST**.
3. Release **BOOT**.
4. Run `pio device list` again, then retry upload with the new port.
5. Press **RST** once after a successful recovery upload.

## Hardware configuration (verified)

The LCD setup is deliberately kept in `src/main.cpp`:

- LCD power: GPIO 15
- Backlight: GPIO 38
- ST7789 chip-select: GPIO 5
- 8-bit bus: DC 7, WR 6, D0–D7 8, 9, 39, 40, 41, 42, 45, 46
- Panel: 170×320 with 35-pixel horizontal offsets, rotated to landscape

The two onboard buttons are available on GPIO 0 and GPIO 14 for participant
projects. Cyberdeck OS uses them for its launcher and Rocket Rush controls.

## Project layout

```text
platformio.ini       pinned board platform and libraries
src/main.cpp         LCD, local Wi-Fi AP, API and design rendering
src/cyberdeck_page.h embedded Cyberdeck OS launcher
src/studio_page.h    embedded offline Display Studio HTML/CSS/JS
src/RocketRush.*     optional example game and two-button controls
include/fonts/       embedded display fonts
AGENTS.md            guidance for Codex
CLAUDE.md            guidance for Claude Code
```

## Safety and privacy

This workshop starter stores only the applied visual design. It does not contain
personal hotspot credentials, API keys, cloud services, analytics, or telemetry.
The shared workshop password contains no personal information. Replace it before
using the device in a public or security-sensitive setting.

