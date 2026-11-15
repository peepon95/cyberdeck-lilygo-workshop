#!/usr/bin/env python3
"""Convert the 12 workshop PNGs to compact 96x96 RGB565 C arrays using macOS sips."""

import pathlib
import struct
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
OUTPUT = ROOT / "include" / "frog_sprites.h"
NAMES = [
    "music", "tired", "cool", "happy_arms", "confused", "sad",
    "star_eyes", "wave", "love", "wink", "sleep", "alert",
]


def bmp_pixels(path: pathlib.Path):
    data = path.read_bytes()
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    if width != 96 or abs(height) != 96 or bpp not in (24, 32):
        raise RuntimeError(f"Unexpected BMP format: {width}x{height} {bpp}bpp")
    stride = ((width * bpp + 31) // 32) * 4
    rows = range(abs(height) - 1, -1, -1) if height > 0 else range(abs(height))
    result = []
    rgb = []
    for row in rows:
        start = offset + row * stride
        for x in range(width):
            b, g, r = data[start + x * (bpp // 8):start + x * (bpp // 8) + 3]
            result.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
            rgb.append((r, g, b))
    return result, rgb


def transparency_mask(rgb):
    """Treat the pale blue image backdrop as transparent, preserving the frog."""
    bits = []
    for r, g, b in rgb:
        is_backdrop = b > 220 and g > 220 and r > 210 and b >= r
        bits.append(0 if is_backdrop else 1)
    packed = []
    for row in range(96):
        for x in range(0, 96, 8):
            byte = 0
            for bit in range(8):
                byte = (byte << 1) | bits[row * 96 + x + bit]
            packed.append(byte)
    return packed


def main():
    lines = ["#pragma once", "", "#include <Arduino.h>", "", "constexpr uint16_t FROG_SPRITE_W = 96;", "constexpr uint16_t FROG_SPRITE_H = 96;", ""]
    with tempfile.TemporaryDirectory() as tmp:
        temp = pathlib.Path(tmp)
        for name in NAMES:
            source = ASSETS / f"frog_{name}.png"
            cropped = temp / f"{name}_cropped.png"
            bmp = temp / f"{name}.bmp"
            subprocess.run(["ffmpeg", "-loglevel", "error", "-y", "-i", str(source), "-vf",
                            "crop=420:420:(iw-420)/2:(ih-420)/2,scale=96:96:flags=neighbor", str(cropped)], check=True)
            subprocess.run(["sips", "-s", "format", "bmp", str(cropped), "--out", str(bmp)], check=True, stdout=subprocess.DEVNULL)
            pixels, rgb = bmp_pixels(bmp)
            lines.append(f"const uint16_t FROG_{name.upper()}[FROG_SPRITE_W * FROG_SPRITE_H] PROGMEM = {{")
            for index in range(0, len(pixels), 16):
                lines.append("  " + ", ".join(f"0x{value:04X}" for value in pixels[index:index + 16]) + ",")
            lines.append("};")
            lines.append(f"const uint8_t FROG_{name.upper()}_MASK[(FROG_SPRITE_W / 8) * FROG_SPRITE_H] PROGMEM = {{")
            mask = transparency_mask(rgb)
            for index in range(0, len(mask), 24):
                lines.append("  " + ", ".join(f"0x{value:02X}" for value in mask[index:index + 24]) + ",")
            lines.append("};")
            lines.append("")
    lines.append("const uint16_t *const FROG_SPRITES[] PROGMEM = {")
    lines.extend(f"  FROG_{name.upper()}," for name in NAMES)
    lines.append("};")
    lines.append("const uint8_t *const FROG_SPRITE_MASKS[] PROGMEM = {")
    lines.extend(f"  FROG_{name.upper()}_MASK," for name in NAMES)
    lines.append("};")
    OUTPUT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
