"""Dump N seconds of serial output from the board (decoded, control chars stripped)."""
import re
import sys
import time

import serial

port = sys.argv[2] if len(sys.argv) > 2 else "COM9"
secs = float(sys.argv[1]) if len(sys.argv) > 1 else 10

s = serial.Serial(port, 115200, timeout=0.5)
s.setDTR(False)
s.setRTS(False)
time.sleep(0.2)
buf = b""
t0 = time.time()
while time.time() - t0 < secs:
    chunk = s.read(8192)
    if chunk:
        buf += chunk
s.close()
text = buf.decode("utf-8", "replace")
text = re.sub(r"\x1b\[[0-9;]*m", "", text)
text = text.replace("\r", "")
# collapse the esptool "Writing at ..." noise
lines = [l for l in text.split("\n") if l.strip()]
print("\n".join(lines[-120:]))
