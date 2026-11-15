#!/usr/bin/env python3
"""Embed the approved frog PNG files for the ESP32-hosted phone dashboard."""

import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "include" / "frog_web_assets.h"
WEB_ASSETS = ROOT / "assets" / "frog_web_transparent"
NAMES = ["music", "tired", "cool", "happy_arms", "confused", "sad",
         "star_eyes", "wave", "love", "wink", "sleep", "alert"]


def main():
    WEB_ASSETS.mkdir(parents=True, exist_ok=True)
    lines = ["#pragma once", "", "#include <Arduino.h>", ""]
    for name in NAMES:
        source = ROOT / "assets" / f"frog_{name}.png"
        web_png = WEB_ASSETS / f"frog_{name}.png"
        subprocess.run([
            "ffmpeg", "-loglevel", "error", "-y", "-i", str(source), "-vf",
            "crop=420:420:(iw-420)/2:(ih-420)/2,colorkey=0xebf5fc:0.012:0.0,scale=256:256:flags=neighbor,format=rgba",
            str(web_png),
        ], check=True)
        data = web_png.read_bytes()
        symbol = f"FROG_WEB_{name.upper()}"
        lines.append(f"const uint8_t {symbol}[] PROGMEM = {{")
        for index in range(0, len(data), 24):
            lines.append("  " + ", ".join(f"0x{value:02X}" for value in data[index:index + 24]) + ",")
        lines.append("};")
        lines.append(f"constexpr size_t {symbol}_LEN = sizeof({symbol});")
        lines.append("")
    lines.append("const uint8_t *const FROG_WEB_SPRITES[] PROGMEM = {")
    lines.extend(f"  FROG_WEB_{name.upper()}," for name in NAMES)
    lines.append("};")
    lines.append("const size_t FROG_WEB_SPRITE_LENGTHS[] PROGMEM = {")
    lines.extend(f"  FROG_WEB_{name.upper()}_LEN," for name in NAMES)
    lines.append("};")
    OUTPUT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
