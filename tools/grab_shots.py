"""Ask the board for page snapshots over serial and save them as PNGs.

The firmware implements a "shot" command: it walks all nine pages, takes an
LVGL snapshot of each and streams it as hex RGB565. This script sends the
command, parses the stream and writes out/shot_pN.png.

    python tools/grab_shots.py [COM9] [outdir]
"""
import os
import sys
import time

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM9"
OUT = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(__file__), "..", "shots")
BAUD = 921600


def main():
    os.makedirs(OUT, exist_ok=True)
    s = serial.Serial(PORT, BAUD, timeout=0.5)
    s.reset_input_buffer()
    time.sleep(0.3)
    s.write(b"s")
    s.flush()

    pages = {}
    page = None
    rows = {}
    hdr = None
    buf = b""
    deadline = time.time() + 180
    done = False
    while time.time() < deadline and not done:
        chunk = s.read(65536)
        if not chunk:
            continue
        buf += chunk
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            txt = line.decode("ascii", "replace").strip("\r")
            if not txt:
                continue
            if txt.startswith("SHOT "):
                parts = txt.split()
                if len(parts) == 5 and parts[2] != "FAIL":
                    page = int(parts[1])
                    hdr = (int(parts[2]), int(parts[3]), int(parts[4]))
                    rows[page] = []
                else:
                    page = None
            elif txt == "END" and page is not None:
                data = b"".join(rows[page])
                w, h, n = hdr
                print(f"page {page}: {w}x{h} {len(data)}/{n} bytes")
                pages[page] = (w, h, data)
                page = None
            elif txt == "SHOTS_DONE":
                done = True
            elif page is not None:
                try:
                    rows[page].append(bytes.fromhex(txt))
                except ValueError:
                    pass
    s.close()

    try:
        from PIL import Image
    except ImportError:
        print("pillow missing")
        return
    for idx, (w, h, data) in sorted(pages.items()):
        img = Image.new("RGB", (w, h))
        px = img.load()
        for y in range(h):
            for x in range(w):
                o = (y * w + x) * 2  # raw little-endian RGB565
                if o + 1 >= len(data):
                    break
                v = data[o] | (data[o + 1] << 8)
                r = ((v >> 11) & 0x1F) * 255 // 31
                g = ((v >> 5) & 0x3F) * 255 // 63
                b = (v & 0x1F) * 255 // 31
                px[x, y] = (r, g, b)
        path = os.path.abspath(os.path.join(OUT, f"shot_p{idx}.png"))
        img.save(path)
        print("saved", path)


if __name__ == "__main__":
    main()
