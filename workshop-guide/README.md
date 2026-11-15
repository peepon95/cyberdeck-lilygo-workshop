# Cyber Deck Build Together tutorial

Interactive workshop guide for 28 November 2026. This website is intentionally separate from the LILYGO firmware repository.

## Preview locally

From this folder, run:

```sh
python3 -m http.server 4173
```

Then open <http://localhost:4173>. A local server is required to test the offline service worker; opening `index.html` directly still shows the guide but will not install offline caching.

## Current repository status

Firmware repository: <https://github.com/peepon95/cyberdeck-lilygo-workshop>

The starter firmware and this guide are published in the same repository as clearly separated folders. The PlatformIO starter remains at the repository root; the website is under `workshop-guide/`.

## Workshop facts reflected in the guide

- LILYGO T-Display-S3, ESP32-S3, 320×170 landscape display
- Two active-low buttons on GPIO 0 and GPIO 14
- Local access point: `LILYGO-SETUP-XXXXXX`
- Local Wi-Fi password: `cyberdeck`
- Wi-Fi setup page: <http://192.168.4.1>
- Mac workflow has been tested on hardware
- Windows instructions are documentation-verified but still require an end-to-end hardware rehearsal

## Files

- `index.html` — page structure and accessible controls
- `styles.css` — responsive presentation, with no external CSS dependency
- `app.js` — tutorial content, OS/tool branching, progress, prompts, troubleshooting
- `sw.js` — offline cache for the local website bundle
- `manifest.webmanifest` — installable-site metadata
- `assets/` — locally bundled workshop imagery

No deployment or firmware-repository push is performed by this website project.
