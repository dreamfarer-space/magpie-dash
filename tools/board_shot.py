"""Drive the board over USB and save screenshots of what it is showing.

Commands travel inside CRC frames like the bridge's (with the bridge running,
raw bytes are dropped as "stray"). The bridge must be stopped first, since it
owns the port; while this runs it stands in for the bridge (answers the
board's data requests and says hello), so the link stays up in the captures.

    py -3.11 tools/board_shot.py --demo --all --out shots          # demo data, every page
    py -3.11 tools/board_shot.py --cmd 4 --cmd o --shot --out shots # agent sheet as it is now
    py -3.11 tools/board_shot.py --tap 120,258 --shot --out shots   # tap, then capture

Steps run in the order given. The firmware dumps the frame as
"OOOOO:<hex>*SS" lines (offset, payload, byte sum); lines that arrive damaged
are re-requested, so a capture is either exact or reported as incomplete.
"""
import argparse
import os
import time

import serial
from PIL import Image

import usb_bridge
from usb_protocol import frame, parse_request

W, H = 480, 272
FRAME_BYTES = W * H * 2
LINE = 64  # payload bytes per dump line


def to_image(data):
    px = bytearray(W * H * 3)
    for i in range(W * H):
        v = data[2 * i] | (data[2 * i + 1] << 8)
        px[3 * i] = ((v >> 11) & 0x1F) * 255 // 31
        px[3 * i + 1] = ((v >> 5) & 0x3F) * 255 // 63
        px[3 * i + 2] = (v & 0x1F) * 255 // 31
    return Image.frombytes("RGB", (W, H), bytes(px))


def parse_line(t):
    """'OOOOO:hex*SS' -> (offset, bytes) or None when damaged."""
    head, sep, rest = t.partition(":")
    payload, star, sums = rest.rpartition("*")
    if not sep or not star or len(head) != 5:
        return None
    try:
        off = int(head, 16)
        data = bytes.fromhex(payload)
        want = int(sums, 16)
    except ValueError:
        return None
    if off % LINE or off + len(data) > FRAME_BYTES or sum(data) & 0xFF != want:
        return None
    if len(data) != min(LINE, FRAME_BYTES - off):
        return None
    return off, data


class Board:
    def __init__(self, port):
        self.s = serial.Serial(port=None, baudrate=921600, timeout=0.2)
        self.s.port = port
        self.s.dtr = False  # toggling DTR/RTS would reset the board
        self.s.rts = False
        self.s.open()
        self.buf = b""
        self.next_hello = 0.0

    def close(self):
        self.s.close()

    def send(self, cmd):
        self.s.write(frame("C", 0, cmd.encode()))
        self.s.flush()

    def lines(self, timeout):
        end = time.time() + timeout
        while time.time() < end:
            if time.time() >= self.next_hello:
                usb_bridge.send_frame(self.s, "H", 0, ('{"epoch":%d}' % time.time()).encode())
                self.next_hello = time.time() + 5
            chunk = self.s.read(65536)
            if chunk:
                self.buf += chunk
            while b"\n" in self.buf:
                line, self.buf = self.buf.split(b"\n", 1)
                req = parse_request(line)
                if req:  # serve it like the bridge would, so the link stays up
                    body, _, _ = usb_bridge.fetch(req[1])
                    usb_bridge.send_frame(self.s, "R", req[0], body)
                    continue
                yield line.decode("ascii", "replace").strip("\r")

    def settle(self, seconds):
        for _ in self.lines(seconds):
            pass

    def tap(self, x, y, settle=0.35):
        self.send(f"@{x},{y}")
        self.settle(settle)

    def _dump(self, cmd, timeout=90):
        """Send s / S; return {page: (bytearray, set of good offsets)}."""
        self.send(cmd)
        pages, page = {}, None
        for t in self.lines(timeout):
            if t.startswith("SHOT "):
                p = t.split()
                page = int(p[1]) if len(p) == 5 else None
                if page is not None:
                    pages[page] = (bytearray(FRAME_BYTES), set())
            elif t == "END":
                page = None
            elif t == "SHOTS_DONE":
                break
            elif page is not None:
                got = parse_line(t)
                if got:
                    off, data = got
                    buf, ok = pages[page]
                    buf[off:off + len(data)] = data
                    ok.add(off)
        return pages

    def shot(self, retries=4):
        """The page in view, as it is now (sheets, bubbles, overlays included)."""
        buf, ok = bytearray(FRAME_BYTES), set()
        need = set(range(0, FRAME_BYTES, LINE))
        for _ in range(retries):
            pages = self._dump("S")
            for b, good in pages.values():
                for off in good - ok:
                    buf[off:off + LINE] = b[off:off + LINE]
                ok |= good
            if ok >= need:
                return to_image(buf)
        raise RuntimeError(f"capture incomplete: {len(need - ok)} of {len(need)} lines missing")

    def shot_page(self, page):
        self.send(str(page))
        self.settle(0.4)
        return self.shot()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default="COM9")
    ap.add_argument("--out", default=os.path.join(os.path.dirname(__file__), "..", "shots"))
    ap.add_argument("--demo", action="store_true", help="load the firmware's demo data first (serial 'm')")
    ap.add_argument("--live", action="store_true", help="leave demo data at the end (serial 'r')")
    ap.add_argument("--all", action="store_true", help="capture every page as shot_p<N>.png")
    ap.add_argument("--cmd", action="append", default=[], help="debug key(s) to send, in order")
    ap.add_argument("--tap", action="append", default=[], help="x,y tap through the input driver")
    ap.add_argument("--shot", action="store_true", help="capture the page in view as shot.png")
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    b = Board(a.port)
    try:
        if a.demo:
            b.send("m")
            b.settle(1.5)
        for c in a.cmd:
            b.send(c)
            b.settle(0.6)
        for t in a.tap:
            x, y = (int(v) for v in t.split(","))
            b.tap(x, y)
        if a.all:
            for page in range(10):
                path = os.path.abspath(os.path.join(a.out, f"shot_p{page}.png"))
                b.shot_page(page).save(path)
                print("saved", path)
            b.send("0")
        if a.shot:
            path = os.path.abspath(os.path.join(a.out, "shot.png"))
            b.shot().save(path)
            print("saved", path)
        if a.live:
            b.send("r")
    finally:
        b.close()


if __name__ == "__main__":
    main()
