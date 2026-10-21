"""Load the firmware's demo data (serial 'm'), then grab all nine pages.

    py -3.11 tools/shot_mock.py [COM9] [outdir]

Writes shot_p0..p8.png plus a 3x3 montage.png into outdir (default shots/mock).
Send 'r' afterwards (or reboot) to return the board to live data.
"""
import os
import sys
import time

import serial
from PIL import Image

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM9"
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), "..", "shots", "mock")


def main():
    os.makedirs(OUT, exist_ok=True)
    s = serial.Serial(PORT, 921600, timeout=0.5)
    time.sleep(0.3)
    s.reset_input_buffer()
    s.write(b"m")
    s.flush()
    time.sleep(1.5)  # let ui_update run
    s.reset_input_buffer()
    s.write(b"s")
    s.flush()

    pages, rows, page, hdr, buf = {}, [], None, None, b""
    deadline = time.time() + 180
    done = False
    while time.time() < deadline and not done:
        chunk = s.read(65536)
        if not chunk:
            continue
        buf += chunk
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            t = line.decode("ascii", "replace").strip("\r")
            if t.startswith("SHOT "):
                p = t.split()
                page = int(p[1]) if len(p) == 5 and p[2] != "FAIL" else None
                hdr = (int(p[2]), int(p[3])) if page is not None else None
                rows = []
            elif t == "END" and page is not None:
                pages[page] = (hdr, b"".join(rows))
                page = None
            elif t == "SHOTS_DONE":
                done = True
            elif page is not None:
                try:
                    rows.append(bytes.fromhex(t))
                except ValueError:
                    pass
    s.close()

    imgs = {}
    for idx, ((w, h), data) in sorted(pages.items()):
        img = Image.new("RGB", (w, h))
        px = img.load()
        for y in range(h):
            for x in range(w):
                o = (y * w + x) * 2
                if o + 1 >= len(data):
                    break
                v = data[o] | (data[o + 1] << 8)
                px[x, y] = (((v >> 11) & 0x1F) * 255 // 31, ((v >> 5) & 0x3F) * 255 // 63,
                            (v & 0x1F) * 255 // 31)
        img.save(os.path.join(OUT, f"shot_p{idx}.png"))
        imgs[idx] = img
    print("pages", sorted(imgs))
    if imgs:
        w, h = 480, 272
        m = Image.new("RGB", (w * 3 + 8, h * 3 + 8), (60, 60, 60))
        for i, im in imgs.items():
            m.paste(im, ((i % 3) * (w + 4), (i // 3) * (h + 4)))
        m.save(os.path.join(OUT, "montage.png"))
        print("saved", os.path.abspath(os.path.join(OUT, "montage.png")))


if __name__ == "__main__":
    main()
