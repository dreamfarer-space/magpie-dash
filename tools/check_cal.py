"""Open the calibration screen, then snapshot page 0 to check it renders.

Sends 'c' (show CAL) then 's' (dump all pages) on the same port session so the
board is not reset in between, and saves only the first page.
"""
import os
import sys
import time

import serial
from PIL import Image

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM9"
OUT = os.path.join(os.path.dirname(__file__), "..", "shots", "cal_check.png")

s = serial.Serial(PORT, 921600, timeout=0.3)
time.sleep(2.5)
s.reset_input_buffer()
s.write(b"c")
s.flush()
time.sleep(1.5)
s.write(b"s")
s.flush()

buf = b""
page = None
hdr = None
rows = []
done = False
t0 = time.time()
while time.time() - t0 < 120 and not done:
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
            print("page", page, "bytes", sum(len(r) for r in rows))
            if page == 0:
                done = True
                break
            page = None
        elif txt == "SHOTS_DONE":
            done = True
        elif page is not None:
            try:
                rows.append(bytes.fromhex(txt))
            except ValueError:
                pass
s.close()

data = b"".join(rows)
if not hdr or not data:
    print("no data")
    sys.exit(1)
w, h = hdr
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
