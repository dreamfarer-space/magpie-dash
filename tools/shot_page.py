"""Wait for a fresh fetch, then dump every page; save the one requested.

Usage: py -3.11 tools/shot_page.py COM9 5 out.png [wait_s]
"""
import os
import sys
import time

import serial
from PIL import Image

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM9"
WANT = int(sys.argv[2]) if len(sys.argv) > 2 else 5
OUT = sys.argv[3] if len(sys.argv) > 3 else os.path.join("shots", "page.png")
WAIT = float(sys.argv[4]) if len(sys.argv) > 4 else 18.0

s = serial.Serial(PORT, 921600, timeout=0.3)
time.sleep(WAIT)          # boot + first fetch
s.reset_input_buffer()
s.write(b"s")
s.flush()

buf = b""
page = None
hdr = None
rows = []
keep = None
t0 = time.time()
while time.time() - t0 < 180:
    chunk = s.read(65536)
    if not chunk:
        continue
    buf += chunk
    while b"\n" in buf:
        line, buf = buf.split(b"\n", 1)
        txt = line.decode("ascii", "replace").strip("\r")
        if txt.startswith("SHOT "):
            parts = txt.split()
            if len(parts) == 5 and parts[2] != "FAIL":
                page = int(parts[1])
                hdr = (int(parts[2]), int(parts[3]))
                rows = []
            else:
                page = None
        elif txt == "END" and page is not None:
            if page == WANT:
                keep = (hdr, b"".join(rows))
                print("captured page", page, "bytes", len(keep[1]))
                page = None
                break
            page = None
        elif txt == "SHOTS_DONE":
            break
        elif page is not None:
            try:
                rows.append(bytes.fromhex(txt))
            except ValueError:
                pass
s.close()

if not keep:
    print("page not captured")
    sys.exit(1)
(w, h), data = keep
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
img.save(OUT)
print("saved", os.path.abspath(OUT))
