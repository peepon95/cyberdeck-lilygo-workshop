# FrogGotchi guide

FrogGotchi is part of the existing Cyberdeck OS firmware. It does not replace
Display Studio, Rocket Rush, Wi-Fi, or their saved settings.

## Open and control it

- On the device: choose **FrogGotchi** in the launcher and press both buttons.
- In a browser: join the device Wi-Fi, then open `http://192.168.4.1`.
- LEFT cycles through care, Focus, Stats, and Background options.
- RIGHT performs the selected action.
- Hold both buttons for 1.8 seconds to return to Cyberdeck OS.

## Pet and focus behaviour

The frog tracks fullness, energy, happiness, health, cleanliness, age, coins,
and completed focus sessions. Companion Mode prevents permanent death. Classic
Mode permits death after prolonged neglect; Love or Medicine revives the frog so workshop
participants cannot become permanently stuck.

A completed focus period awards five coins, improves happiness, starts a
five-minute break, rotates the motivational quote, and advances accessory
rewards. Accessories unlock after 2, 5, and 10 completed focus sessions.

All pet data uses the separate `frogpet` Preferences namespace. The browser
supplies current epoch time when it opens so elapsed care can be applied after
power loss. The ESP32-S3 does not have a battery-backed real-time clock, so
offline elapsed time is updated the next time the FrogGotchi page is opened.
An in-progress Pomodoro is intentionally not restored after a reboot.

## Artwork

The twelve supplied PNG files live in `assets/`. Run this on macOS after
replacing any artwork:

```sh
python3 tools/generate_frog_sprites.py
```

The script uses the built-in `sips` utility to create the compact RGB565 header
used by the LCD firmware. The generated header is committed so participants do
not need to run the converter just to build or flash the project.

The approved Cozy Room, Study Desk and Frog Pond source artwork is stored in
`assets/backgrounds/source`. Exact 320×170 previews are generated without
stretching, then embedded as RGB565 LCD data and local PNG web assets:

```sh
python3 tools/prepare_background_previews.py
python3 tools/generate_background_assets.py
python3 tools/generate_frog_sprites.py
python3 tools/generate_frog_web_assets.py
```

The mobile interface does not need internet access or a CDN. Its six sections
are My Frog, Focus, Backgrounds, Rewards, Pet Stats and Settings. The original
Cyberdeck OS launcher remains available at `http://192.168.4.1/os`.
