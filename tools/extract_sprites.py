"""Extract pixel-art sprites from the 9-page design and emit LVGL C arrays.

Every sprite is the main blob (plus satellites >= 8% of its size) inside a
search box, so crops are exact and reproducible.

Usage:  python extract_sprites.py            # report bboxes + ASCII proof
        python extract_sprites.py --write    # also write assets_sprites.h
"""
import os
import sys
from collections import Counter, deque

import numpy as np
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PAGES = os.path.join(ROOT, "real", "pages")
OUT = os.path.join(ROOT, "firmware", "magepie_dash", "assets_sprites.h")

# name: (page, x0, y0, x1, y1)
SPRITES = {
    "mascot_small": (1, 40, 85, 235, 200),
    "bird":         (1, 275, 60, 465, 200),
    "mascot_big":   (2, 30, 90, 226, 230),
    "star_think":   (2, 144, 4, 236, 90),
    "bubble_code":  (3, 144, 2, 236, 92),
    "bubble_alert": (4, 144, 8, 236, 96),
    "home_icon":    (2, 2, 2, 42, 40),
    "sparkle":      (1, 158, 56, 212, 92),
    # agent-list row icons, page 5 (first row y=56, pitch 32)
    "ic_agent0":    (5, 4, 54, 30, 82),
    "ic_agent1":    (5, 4, 86, 30, 114),
    "ic_agent2":    (5, 4, 118, 30, 146),
    "ic_agent3":    (5, 4, 150, 30, 178),
    "ic_agent4":    (5, 4, 182, 30, 210),
    "ic_agent5":    (5, 4, 214, 30, 242),
    "ic_agent6":    (5, 4, 246, 30, 274),
}


def load(page):
    return np.array(Image.open(os.path.join(PAGES, f"page{page}.png")).convert("RGB")).astype(int)


def components(mask):
    h, w = mask.shape
    seen = np.zeros_like(mask, dtype=bool)
    comps = []
    for y in range(h):
        for x in range(w):
            if not mask[y, x] or seen[y, x]:
                continue
            q = deque([(y, x)])
            seen[y, x] = True
            pts = []
            while q:
                cy, cx = q.popleft()
                pts.append((cy, cx))
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        ny, nx = cy + dy, cx + dx
                        if 0 <= ny < h and 0 <= nx < w and mask[ny, nx] and not seen[ny, nx]:
                            seen[ny, nx] = True
                            q.append((ny, nx))
            comps.append(pts)
    return comps


def extract(page, box):
    a = load(page)
    x0, y0, x1, y1 = box
    sub = a[y0:y1, x0:x1]
    # background = the box's own corner pixel (page bg or card bg);
    # a histogram would pick the mascot body in tight boxes.
    bg = sub[0, 0].copy()
    mask = np.abs(sub - bg).sum(axis=2) > 40
    comps = components(mask)
    if not comps:
        return None
    comps.sort(key=len, reverse=True)
    keep = np.zeros(mask.shape, dtype=bool)
    big = len(comps[0])
    for c in comps:
        if len(c) >= max(6, big * 0.08):
            for cy, cx in c:
                keep[cy, cx] = True
    ys, xs = np.where(keep)
    bbox = (int(xs.min()), int(ys.min()), int(xs.max()) + 1, int(ys.max()) + 1)
    return bbox, sub, keep, tuple(int(v) for v in bg)


def silhouette(keep, bbox, step=2):
    x0, y0, x1, y1 = bbox
    return ["".join("#" if keep[y, x] else "." for x in range(x0, x1, step))
            for y in range(y0, y1, step)]


def rgb565(c):
    r, g, b = int(c[0]), int(c[1]), int(c[2])
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)


def main(write=False):
    chunks = []
    for name, (page, *box) in SPRITES.items():
        res = extract(page, box)
        if not res:
            print(name, "EMPTY")
            continue
        bbox, sub, keep, bg = res
        x0, y0, x1, y1 = bbox
        w, h = x1 - x0, y1 - y0
        print(f"\n{name}: page{page} bbox=({x0},{y0})-({x1},{y1}) {w}x{h} bg={bg} px={int(keep.sum())}")
        for line in silhouette(keep, bbox, 2):
            print("   ", line)
        if write:
            data = bytearray()
            for yy in range(y0, y1):
                for xx in range(x0, x1):
                    if keep[yy, xx]:
                        c = rgb565(sub[yy, xx])
                        data += bytes((c & 0xFF, (c >> 8) & 0xFF, 255))
                    else:
                        data += b"\x00\x00\x00"
            body = ",".join(str(b) for b in data)
            chunks.append(
                f"static const uint8_t {name}_map[] = {{{body}}};\n"
                f"static lv_img_dsc_t {name};\n"
                f"static void {name}_init(void) {{\n"
                f"  {name}.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;\n"
                f"  {name}.header.always_zero = 0;\n"
                f"  {name}.header.reserved = 0;\n"
                f"  {name}.header.w = {w};\n"
                f"  {name}.header.h = {h};\n"
                f"  {name}.data_size = {len(data)};\n"
                f"  {name}.data = {name}_map;\n"
                f"}}\n\n"
            )
    if write:
        os.makedirs(os.path.dirname(OUT), exist_ok=True)
        calls = "".join(f"  {name}_init();\n" for name in SPRITES)
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("// Generated by tools/extract_sprites.py - do not edit\n")
            f.write("#pragma once\n#include <lvgl.h>\n\n")
            f.write("".join(chunks))
            f.write("static inline void assets_init(void) {\n" + calls + "}\n")
        print("\nwrote", OUT, os.path.getsize(OUT), "bytes")


if __name__ == "__main__":
    main("--write" in sys.argv)
