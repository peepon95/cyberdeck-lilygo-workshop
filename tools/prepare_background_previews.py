#!/usr/bin/env python3
"""Centre-crop approved source art to 32:17 and scale to 320x170 with hard pixels."""

import json
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets" / "backgrounds" / "source"
PREVIEWS = ROOT / "assets" / "backgrounds" / "previews"
NAMES = ["cozy_room", "study_desk", "frog_pond"]


def dimensions(path):
    result = subprocess.run([
        "ffprobe", "-v", "error", "-select_streams", "v:0",
        "-show_entries", "stream=width,height", "-of", "json", str(path),
    ], check=True, capture_output=True, text=True)
    stream = json.loads(result.stdout)["streams"][0]
    return stream["width"], stream["height"]


def main():
    PREVIEWS.mkdir(parents=True, exist_ok=True)
    for name in NAMES:
        source = SOURCE / f"{name}_source.png"
        width, height = dimensions(source)
        target_ratio = 320 / 170
        if width / height > target_ratio:
            crop_h = height
            crop_w = int(height * target_ratio) // 2 * 2
        else:
            crop_w = width // 2 * 2
            crop_h = int(width / target_ratio) // 2 * 2
        output = PREVIEWS / f"{name}_320x170.png"
        vf = f"crop={crop_w}:{crop_h}:(iw-{crop_w})/2:(ih-{crop_h})/2,scale=320:170:flags=neighbor"
        subprocess.run(["ffmpeg", "-loglevel", "error", "-y", "-i", str(source), "-vf", vf, str(output)], check=True)


if __name__ == "__main__":
    main()
