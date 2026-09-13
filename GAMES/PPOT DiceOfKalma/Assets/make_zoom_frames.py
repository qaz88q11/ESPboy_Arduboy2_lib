#!/usr/bin/env python3
"""
make_zoom_frames.py

Takes a base image and generates a sequence of "zoom in" frames.
The zoom INCREMENT itself grows each frame (e.g. +10%, then +12%,
then +14%, ...) so the zoom accelerates smoothly instead of jumping
in big flat steps. Starts at 100% and continues until the zoom level
passes END_PCT. Every frame is:
  - scaled up from the original by the current zoom factor
  - cropped back down to the target size (64 x 128), centered
  - converted to pure black & white, with black made transparent

Frames are saved as Zoom_001.png, Zoom_002.png, ... in the output folder.

Usage:
    python3 make_zoom_frames.py [input_image] [output_dir]

Defaults:
    input_image = Zoom_Base.png
    output_dir  = zoom_frames
"""

import os
import sys
from PIL import Image

# ---- Config ----------------------------------------------------------
TARGET_W, TARGET_H = 64, 128   # final frame size (Arduboy screen dims)
START_PCT = 100                # starting zoom level (%)
STEP_START_PCT = 10            # increment used for the FIRST step (%)
STEP_GROWTH_PCT = 2            # how much the increment grows each frame (%)
END_PCT = 3000                 # stop once zoom level exceeds this (%)
THRESHOLD = 128                # grayscale cutoff for pure black/white
CENTER_X = 32                  # zoom-in point: pixels in from the left (base image coords)
CENTER_Y = 22                  # zoom-in point: pixels down from the top (base image coords)
# -----------------------------------------------------------------------


def to_pure_bw(img: Image.Image) -> Image.Image:
    """
    Convert an image to pure black & white (no gray/anti-aliasing),
    then make the black pixels transparent. White pixels stay opaque.
    Returns an RGBA image.
    """
    gray = img.convert("L")
    bw = gray.point(lambda p: 255 if p >= THRESHOLD else 0)
    rgba = bw.convert("RGBA")
    # Alpha = the same as the pixel value: white(255) -> opaque, black(0) -> transparent
    rgba.putalpha(bw)
    return rgba


def make_zoom_frame(base: Image.Image, pct: int) -> Image.Image:
    """
    Scale `base` up by `pct` percent, then crop back to
    TARGET_W x TARGET_H such that the point (CENTER_X, CENTER_Y) in
    the ORIGINAL base image stays fixed at that exact same pixel
    location (CENTER_X, CENTER_Y) in the OUTPUT frame as zoom
    increases — i.e. it's the anchor the image scales around, not
    necessarily the middle of the frame.
    """
    factor = pct / 100.0
    new_w = max(TARGET_W, round(base.width * factor))
    new_h = max(TARGET_H, round(base.height * factor))

    scaled = base.resize((new_w, new_h), Image.LANCZOS)

    # Where the anchor point lands in the scaled image
    scaled_center_x = CENTER_X * factor
    scaled_center_y = CENTER_Y * factor

    # Crop box positioned so that point ends up at (CENTER_X, CENTER_Y)
    # in the TARGET_W x TARGET_H output frame (not the frame's middle).
    left = round(scaled_center_x - CENTER_X)
    top = round(scaled_center_y - CENTER_Y)
    right = left + TARGET_W
    bottom = top + TARGET_H

    # Pad with black on any side the ideal box overhangs (shouldn't
    # normally happen for factor >= 1, but keeps this robust).
    pad_left = max(0, -left)
    pad_top = max(0, -top)
    pad_right = max(0, right - new_w)
    pad_bottom = max(0, bottom - new_h)

    if pad_left or pad_top or pad_right or pad_bottom:
        padded = Image.new(
            "RGB",
            (new_w + pad_left + pad_right, new_h + pad_top + pad_bottom),
            (0, 0, 0),
        )
        padded.paste(scaled, (pad_left, pad_top))
        scaled = padded
        left += pad_left
        top += pad_top
        right += pad_left
        bottom += pad_top

    cropped = scaled.crop((left, top, right, bottom))
    return to_pure_bw(cropped)


def main():
    input_path = sys.argv[1] if len(sys.argv) > 1 else "Zoom_Base.png"
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "zoom_frames"

    os.makedirs(output_dir, exist_ok=True)

    base = Image.open(input_path).convert("RGBA")
    # Flatten onto white background in case of transparency, before B&W pass
    flat = Image.new("RGBA", base.size, (255, 255, 255, 255))
    flat.paste(base, (0, 0), base)
    base = flat.convert("RGB")

    pct = START_PCT
    step = STEP_START_PCT
    seq = 1
    saved = []
    while pct <= END_PCT:
        frame = make_zoom_frame(base, pct)
        filename = f"Zoom_{seq:03d}.png"
        out_path = os.path.join(output_dir, filename)
        frame.save(out_path)
        saved.append((filename, pct))
        seq += 1
        pct += step
        step += STEP_GROWTH_PCT

    print(f"Saved {len(saved)} frames to '{output_dir}/':")
    for filename, pct in saved:
        print(f"  {filename}  ({pct}% zoom)")


if __name__ == "__main__":
    main()
