#!/usr/bin/env python3
"""Generate LCD RGB565 and local-web PNG arrays from approved 320x170 backgrounds."""

import pathlib
import struct
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
PREVIEWS = ROOT / "assets" / "backgrounds" / "previews"
OUTPUT = ROOT / "include" / "frog_backgrounds.h"
NAMES = ["cozy_room", "study_desk", "frog_pond"]


def bmp_pixels(path):
    data = path.read_bytes()
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    if width != 320 or abs(height) != 170 or bpp not in (24, 32):
        raise RuntimeError(f"Unexpected BMP format: {width}x{height} {bpp}bpp")
    stride = ((width * bpp + 31) // 32) * 4
    rows = range(abs(height) - 1, -1, -1) if height > 0 else range(abs(height))
    pixels = []
    for row in rows:
        start = offset + row * stride
        for x in range(width):
            b, g, r = data[start + x * (bpp // 8):start + x * (bpp // 8) + 3]
            pixels.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    return pixels


def add_array(lines, ctype, name, values, width):
    lines.append(f"const {ctype} {name}[] PROGMEM = {{")
    fmt = "0x{:04X}" if ctype == "uint16_t" else "0x{:02X}"
    for index in range(0, len(values), width):
        lines.append("  " + ", ".join(fmt.format(v) for v in values[index:index + width]) + ",")
    lines.append("};")
    lines.append("")


def main():
    lines = ["#pragma once", "", "#include <Arduino.h>", "",
             "constexpr uint16_t FROG_BG_W = 320;", "constexpr uint16_t FROG_BG_H = 170;", ""]
    with tempfile.TemporaryDirectory() as tmp:
        for name in NAMES:
            png = PREVIEWS / f"{name}_320x170.png"
            bmp = pathlib.Path(tmp) / f"{name}.bmp"
            subprocess.run(["sips", "-s", "format", "bmp", str(png), "--out", str(bmp)],
                           check=True, stdout=subprocess.DEVNULL)
            add_array(lines, "uint16_t", f"FROG_BG_{name.upper()}", bmp_pixels(bmp), 16)
            add_array(lines, "uint8_t", f"FROG_BG_{name.upper()}_PNG", list(png.read_bytes()), 24)
            lines.append(f"constexpr size_t FROG_BG_{name.upper()}_PNG_LEN = sizeof(FROG_BG_{name.upper()}_PNG);")
            lines.append("")
    lines.append("const uint16_t *const FROG_BACKGROUNDS[] PROGMEM = {")
    lines.extend(f"  FROG_BG_{name.upper()}," for name in NAMES)
    lines.append("};")
    OUTPUT.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
