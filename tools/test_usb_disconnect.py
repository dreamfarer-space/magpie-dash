"""Regression: closing the host USB port must not freeze the board.

Before the fix, an unread CDC TX ring made Serial.printf block the fetch task and
the UI loop, so the screen silently froze until a hardware reset. This test opens
the link, drops it without a goodbye, waits longer than the TX ring can absorb,
then reopens and requires fresh board output.
"""
import json, time, sys, serial
from usb_protocol import frame

PORT = sys.argv[1] if len(sys.argv) > 1 else 'COM9'
DEAD = float(sys.argv[2]) if len(sys.argv) > 2 else 90.0

def open_port():
    s = serial.Serial(port=None, baudrate=921600, timeout=.1)
    s.port = PORT; s.dtr = False; s.rts = False; s.open()
    return s

def pump(s, seconds, expect):
    raw = b''; end = time.time() + seconds
    while time.time() < end:
        raw += s.read(8192)
        if expect in raw:
            return raw
    return raw

s = open_port()
s.write(frame('H', 0, json.dumps({'epoch': int(time.time())}).encode()))
# The board only prints "link up" on a transition; if it was already linked it
# simply answers requests. Any board output proves the link is alive.
first = pump(s, 20, b'[usb] link up')
ok1 = len(first) > 0
print('phase1 board_talking', ok1, 'link_up', b'[usb] link up' in first, flush=True)
s.close()                      # hard drop, no cleanup, like killing the bridge
print(f'phase2 port closed, waiting {DEAD:.0f}s with nobody reading', flush=True)
time.sleep(DEAD)

s = open_port()
s.write(frame('H', 0, json.dumps({'epoch': int(time.time())}).encode()))
back = pump(s, 30, b'[usb] link up')
s.close()
relinked = b'[usb] link up' in back
ok3 = relinked or b'[hb]' in back or b'[gw]' in back
print('phase3 relinked', relinked, 'alive', ok3, 'bytes', len(back), flush=True)
print('RESULT', 'PASS' if ok1 and ok3 else 'FAIL', flush=True)
sys.exit(0 if ok1 and ok3 else 1)
