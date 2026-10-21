"""Record the README demo: a scripted tour on the real board, cut like a screen
recording whose camera follows the finger.

    py -3.11 tools/demo_reel.py capture            # drive the board, save exact frames
    py -3.11 tools/demo_reel.py render             # frames -> docs/demo.mp4 + docs/demo.gif

capture: loads the demo data, then for every step taps through the LVGL input
driver ('@x,y' frame) and dumps the panel (tools/board_shot.py), so every
frame is what the panel really showed. Stop the USB bridge first.

render: no captions. The camera eases toward each tap, a touch marker presses
and ripples, the screen cuts to the captured result (page switches take ~50 ms
on the device, so there is nothing to tween), and the camera pulls back out
at the end. Needs Pillow, numpy and ffmpeg on PATH.
"""
import argparse
import json
import math
import os
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
WORK = os.path.join(ROOT, "build_logs", "demo_reel")
DOCS = os.path.join(ROOT, "docs")

# (tap x, y on the 480x272 panel or None, extra serial keys, seconds to hold)
STEPS = [
    (None, "0", 1.6),            # home
    ((150, 175), "", 1.4),        # agent pane -> its state page
    ((96, 176), "", 1.3),         # pip -> "coding"
    ((120, 258), "", 1.4),        # tab: Agent
    ((240, 92), "", 1.6),         # row: Codex -> detail sheet
    ((62, 100), "", 1.5),         # poke the avatar
    ((332, 210), "", 0.9),        # close
    ((200, 258), "", 1.6),        # tab: quota
    ((280, 258), "", 1.6),        # tab: usage
    ((360, 258), "", 1.4),        # tab: routes
    ((440, 30), "", 1.6),         # expand the log
    ((440, 258), "", 1.2),        # tab: settings
    ((240, 216), "", 1.4),        # connection panel
    ((40, 258), "", 1.2),         # tab: home
    ((48, 70), "", 2.0),          # poke
]


# ---------------------------------------------------------------- capture
def capture(port):
    sys.path.insert(0, HERE)
    from board_shot import Board

    os.makedirs(WORK, exist_ok=True)
    b = Board(port)
    meta = []
    try:
        b.send("m")  # demo data: no real accounts in the docs
        b.settle(1.5)
        for i, (tap, keys, hold) in enumerate(STEPS):
            for k in keys:
                b.send(k)
                b.settle(0.5)
            if tap:
                b.tap(*tap, settle=0.45)
            path = os.path.join(WORK, f"f{i:02d}.png")
            b.shot().save(path)
            meta.append({"frame": path, "tap": tap, "hold": hold})
            print("step", i, tap, "->", path)
        b.send("0")
    finally:
        b.close()
    with open(os.path.join(WORK, "steps.json"), "w") as f:
        json.dump(meta, f, indent=1)


# ---------------------------------------------------------------- render
OUT_W, OUT_H, FPS = 1280, 720, 30
S = 4                       # panel pixels -> world pixels (nearest: crisp pixel font)
PAD = 36 * S // 2           # bezel thickness in world pixels
BG = (13, 16, 20)


def ease(t):
    t = max(0.0, min(1.0, t))
    return t * t * (3 - 2 * t)


def world_geometry(screen):
    """Canvas size (16:9, so a wide shot never leaves it) and the bezel origin."""
    sw, sh = screen.width * S, screen.height * S
    w, h = sw + 2 * PAD, sh + 2 * PAD
    cw = max(w + 400, int((h + 400) * OUT_W / OUT_H))
    ch = max(h + 400, int(cw * OUT_H / OUT_W))
    return cw, ch, (cw - w) // 2, (ch - h) // 2, w, h


def device_world(screen):
    """Panel image inside a simple bezel, at S x scale, on the background."""
    cw, ch, ox, oy, w, h = world_geometry(screen)
    img = Image.new("RGB", (cw, ch), BG)
    shadow = Image.new("L", img.size, 0)
    ImageDraw.Draw(shadow).rounded_rectangle((ox, oy + 30, ox + w, oy + 30 + h), 60, fill=150)
    shadow = shadow.filter(ImageFilter.GaussianBlur(40))
    img.paste(Image.new("RGB", img.size, (4, 5, 7)), (0, 0), shadow)
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((ox, oy, ox + w, oy + h), 44, fill=(28, 31, 36), outline=(48, 53, 61), width=4)
    img.paste(screen.resize((screen.width * S, screen.height * S), Image.NEAREST), (ox + PAD, oy + PAD))
    return img


def render(gif_width):
    with open(os.path.join(WORK, "steps.json")) as f:
        steps = json.load(f)
    screens = [Image.open(s["frame"]).convert("RGB") for s in steps]
    worlds = [device_world(sc) for sc in screens]
    ww, wh, ox, oy, dw, dh = world_geometry(screens[0])
    wide_z = min(OUT_W / (dw + 160), OUT_H / (dh + 160))  # whole device, a little air
    close_z = wide_z * 1.35

    def to_world(x, y):
        return ox + PAD + x * S, oy + PAD + y * S

    # camera keyframes: (time, cx, cy, zoom); marker events: (time, x, y)
    keys, taps, cuts = [], [], []
    t = 0.0
    keys.append((t, ww / 2, wh / 2, wide_z))
    t += steps[0]["hold"]
    keys.append((t, ww / 2, wh / 2, wide_z))
    for i, s in enumerate(steps[1:], start=1):
        x, y = to_world(*s["tap"])
        # keep the framed area inside the device
        half_w, half_h = OUT_W / close_z / 2, OUT_H / close_z / 2
        cx = min(max(x, ox + half_w), ox + dw - half_w)
        cy = min(max(y, oy + half_h), oy + dh - half_h)
        t += 0.55
        keys.append((t, cx, cy, close_z))   # camera arrives
        taps.append((t + 0.05, x, y))
        cuts.append((t + 0.17, i))          # result on screen after the press
        t += 0.17 + s["hold"]
        keys.append((t, cx, cy, close_z))
    t += 0.7
    keys.append((t, ww / 2, wh / 2, wide_z))
    t += 1.0
    keys.append((t, ww / 2, wh / 2, wide_z))
    total = t

    def camera(at):
        for (t0, x0, y0, z0), (t1, x1, y1, z1) in zip(keys, keys[1:]):
            if at <= t1:
                k = ease((at - t0) / (t1 - t0)) if t1 > t0 else 1
                # zoom in log space so pans feel even
                return x0 + (x1 - x0) * k, y0 + (y1 - y0) * k, math.exp(math.log(z0) + (math.log(z1) - math.log(z0)) * k)
        return keys[-1][1:]

    def frame_index(at):
        idx = 0
        for tc, i in cuts:
            if at >= tc:
                idx = i
        return idx

    mp4 = os.path.join(DOCS, "demo.mp4")
    os.makedirs(DOCS, exist_ok=True)
    ff = subprocess.Popen(
        ["ffmpeg", "-y", "-loglevel", "error", "-f", "rawvideo", "-pix_fmt", "rgb24",
         "-s", f"{OUT_W}x{OUT_H}", "-r", str(FPS), "-i", "-",
         "-c:v", "libx264", "-preset", "slow", "-crf", "18", "-pix_fmt", "yuv420p",
         "-movflags", "+faststart", mp4],
        stdin=subprocess.PIPE)
    n = int(total * FPS)
    for f in range(n):
        at = f / FPS
        cx, cy, z = camera(at)
        src = worlds[frame_index(at)]
        bw, bh = OUT_W / z, OUT_H / z
        box = (cx - bw / 2, cy - bh / 2, cx + bw / 2, cy + bh / 2)
        out = src.resize((OUT_W, OUT_H), Image.NEAREST, box=box)
        d = ImageDraw.Draw(out, "RGBA")
        for tt, x, y in taps:
            dt = at - tt
            if -0.45 < dt < 0.6:
                sx, sy = (x - box[0]) * z, (y - box[1]) * z
                if dt < 0:  # finger approaching
                    a = int(200 * ease(1 + dt / 0.45))
                    r = 22
                else:       # press, then a ring spreading out
                    a = int(220 * (1 - ease(dt / 0.6)))
                    r = 22 - 6 * ease(min(dt / 0.12, 1)) + 4 * ease(max(dt - 0.12, 0) / 0.48)
                    rr = 22 + 50 * ease(dt / 0.6)
                    d.ellipse((sx - rr, sy - rr, sx + rr, sy + rr), outline=(255, 255, 255, a // 2), width=3)
                d.ellipse((sx - r, sy - r, sx + r, sy + r), fill=(255, 255, 255, a // 2),
                          outline=(255, 255, 255, a), width=3)
        ff.stdin.write(out.tobytes())
    ff.stdin.close()
    ff.wait()
    print("saved", mp4, f"{total:.1f} s")

    gif = os.path.join(DOCS, "demo.gif")
    vf = (f"fps=15,scale={gif_width}:-1:flags=neighbor,split[a][b];"
          "[a]palettegen=max_colors=96:stats_mode=diff[p];[b][p]paletteuse=dither=none:diff_mode=rectangle")
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", "-i", mp4, "-vf", vf, gif], check=True)
    print("saved", gif, os.path.getsize(gif) // 1024, "KB")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("stage", choices=["capture", "render", "all"])
    ap.add_argument("--port", default="COM9")
    ap.add_argument("--gif-width", type=int, default=800)
    a = ap.parse_args()
    if a.stage in ("capture", "all"):
        capture(a.port)
    if a.stage in ("render", "all"):
        render(a.gif_width)


if __name__ == "__main__":
    main()
