<div align="center">

# magpie-dash

**A terminal-style 480×272 touch dashboard for the magpie LLM gateway.**
ESP32-S3 · LVGL 8 · one USB cable or plain WiFi

*Which agent is working, how much quota is left, what today cost — one glance, no browser tab.*

[![CI](https://github.com/dreamfarer-space/magpie-dash/actions/workflows/ci.yml/badge.svg)](https://github.com/dreamfarer-space/magpie-dash/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-7B6DFF.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-ESP32--S3-DE7552.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![UI](https://img.shields.io/badge/UI-LVGL%208-39DE83.svg)](https://lvgl.io)
[![Font](https://img.shields.io/badge/font-Ark%20Pixel-4A90D9.svg)](https://github.com/TakWolf/ark-pixel-font)

**English** · [简体中文](README.zh-CN.md)

</div>

## Features

<p align="center">
  <img src="docs/demo.gif" alt="Board walkthrough: Home → Status → Agent list and details → `额度` → `用量` → Route log → Settings" width="800">
</p>
<p align="center"><a href="docs/demo.mp4">High-definition MP4</a>: frame-by-frame captures rendered on the physical board (demo data), tracking each tap.</p>

Terminal style: the top row is a shell prompt (`user@magpie:~/agents$`, blinking cursor, link status, time). The bottom row provides htop-style function keys (`1 Home` ... `6 Settings`). The center contains bordered wireframe panes with titles. Typography uses **Ark Pixel monospace** (6 px per half-width cell, 12 px per full-width cell). Every column aligns to this 80-column grid across rows and pages. Color represents state: green = normal / remaining quota, amber = busy / low quota, red = error / exhausted, plus brand colors for individual agents.

![Overview of screens](docs/overview.png)

| Page | Content |
|---|---|
| **Home** | Left: active agent (avatar, model, status, dual quota bars, today / 30-day calls, reset countdown). Right: today's calls and tokens, cost and token chart for the selected usage period (30 d by default), and the number of distinct sessions in the fetched request rows. |
| **Status pages ×3** | Thinking / Writing code / Awaiting confirmation: latency, model, and quota consumption of the latest request; switch between them via three small bars beneath the avatar. |
| **Agent** | All agents managed by magpie: name, model, status (Working / Finished / Idle / Unused), 30-day calls. Tap a row to view details or enable "Follow". |
| **`额度`** | One pane per upstream account. Each quota window displays a `■` progress bar, percentage, and reset time. |
| **`用量`** | Today / 7 days / 30 days / All: tokens (input / output / cache), requests and failures, cost and average price, cache hit rate, time-bucketed bar chart. |
| **Routes** | Topology of the latest request (`agent ──▶ magpie ──▶ upstream account`) above a scrollable request log. Tap the top bar to expand into a full-page log. |
| **Settings** | Display name (on-screen keyboard), remaining / used quota toggle, refresh interval, avatar animation, hide cost toggle, connection mode (USB / WiFi). |

| | | |
|---|---|---|
| ![Home](docs/screen-home.png) | ![Agent list](docs/screen-agents.png) | ![Agent details](docs/screen-agent-sheet.png) |
| ![`额度`](docs/screen-quota.png) | ![`用量`](docs/screen-usage.png) | ![Routes](docs/screen-routes.png) |

### Interaction

- Bottom function keys switch pages on **finger press-down** rather than release. Latency from press-down to full page render is 51 ms (measured on board, see below).
- Tap the Home left pane to open Status pages; tap the right pane to open the Agent list. Tap the shell prompt (showing your name) to open Settings.
- Tap the avatar: the sprite jumps and displays its catchphrase (e.g. Claude Code "Clauding...", Codex "> codex exec"); tapping 5 times triggers a complaint.
- "Follow" in Agent details pins the Home and Status pages to that agent, reverting to "follow latest request" after 10 minutes.
- Long-press the top prompt bar to enter two-point touch calibration.

## Hardware

| Item | Specification |
|---|---|
| Board | Guition **JC4827W543** (4.3" 480×272 IPS) |
| SoC | ESP32-S3-WROOM-1 N4R8: 4 MB flash, 8 MB PSRAM |
| Display | NV3041A, QSPI: CS45 / SCK47 / D0-21 / D1-48 / D2-40 / D3-39, backlight GPIO1, TE GPIO0 |
| Touch | GT911, I²C: SCL4 / SDA8 / INT3 / RST38, address 0x5D |
| Debug | On-board USB-Serial/JTAG (no CH340), `CDCOnBoot=cdc`, 921600 baud |

> GPIO0 is connected to the LCD TE signal, **not a push button**. This board has no usable BOOT button; navigation relies entirely on touch.

## Quick start

Toolchain (matching CI in [`.github/workflows/ci.yml`](.github/workflows/ci.yml)): arduino-cli, esp32 core **3.3.12**, lvgl **8.3.7**, ArduinoJson **7.4.3**, GFX Library for Arduino **1.6.9**. Place [`firmware/lv_conf.h`](firmware/lv_conf.h) in the Arduino `libraries/` directory at the same level as `lvgl/`.

```bash
bash flash.sh            # Compile only
bash flash.sh flash      # Compile + flash + capture 10 seconds of serial logs
```

The top of `flash.sh` defines local paths for arduino-cli and data directories; modify those lines when moving to another machine. During flashing, the script stops the auto-started USB bridge task and restarts it after flashing completes.

## Connecting to magpie

### USB cable (default)

Plug the board into the computer. USB supplies power and carries data simultaneously. No WiFi or open network ports required; the computer continues using its existing network configuration. Run the bridge on the host (reads magpie on 127.0.0.1 only):

```bash
py -3.11 tools/usb_bridge.py --port COM9                       # Run in foreground
powershell -File tools/bridge_autostart.ps1 -Action install    # run at Windows logon (pythonw, no console; installing does not start it)
powershell -File tools/bridge_autostart.ps1 -Action status
```

- Discovers the board by VID/PID `0x303a:0x1001`, reconnecting even if the COM port changes. Duplicate instances are blocked by a named mutex.
- Status is written to `build_logs/usb_bridge_status.json`; logs are written to `build_logs/usb_bridge.log`.
- Protocol uses length-prefixed CRC32 frames (`!MG1 <H|R|C> <seq> <len> <crc>\n` + body). The request whitelist permits read-only dashboard endpoints only. While the USB handshake is active, bytes outside a frame are discarded, so line noise cannot fire a debug action. When the link is inactive the firmware falls back to raw single-key serial commands.

### WiFi (not wired up in this build)

The connection panel offers WiFi, and the networking code is compiled in, but the default build runs with `USB_LINK_ONLY 1` in `firmware/magepie_dash/config.h`, which skips `netcfg_begin()` / `netcfg_loop()` in `setup()`/`loop()`. Switching the panel to WiFi therefore raises the access point but never serves the setup page or advances the station, so **plan on the USB link for now**. Treat the notes below as what the WiFi path needs once that guard is removed and the path is tested on hardware.

What it does when enabled: the board joins a 2.4 GHz network and talks HTTP to the machine running magpie. First-time setup runs on the device itself: an access point `magpie-dash-setup` (password `magpie123`) at `http://192.168.4.1`, where the SSID, password, gateway address and port are entered separately.

Two constraints:

1. **magpie listens on 127.0.0.1 only.** Run the relay on the host (`python tools/gw_relay.py 3426 127.0.0.1 3425`), then enter the computer IP in the gateway field and `3426` in the port field.
2. **The ESP32-S3 is 2.4 GHz only, and this firmware does not implement 802.1X enterprise authentication.** Campus-style networks will not work; use a phone hotspot or a normal router (`tools/start_hotspot.ps1` starts a Windows hotspot).

Joining is non-blocking, and the configuration lives in NVS, so a re-flash keeps it.

## Where the data comes from

| Request | Fields used |
|---|---|
| `/v1/magpie/quotas` | `provider/name/plan/user`, `windows[].name/used/remaining/resetsAt` |
| `/v1/magpie/usage?period=` | `calls/errors/input/output/cache_read/cost`, `series[]` |
| `/v1/magpie/usage/requests?period=&limit=12` | `rows[]` (agent / provider / model / stop / token / duration / status), `total` |
| `/dash/agents` (served locally by bridge / relay) | Agent list: `id/name/model/calls/tokens/cost/today/last` |

- magpie accepts only `today/7d/30d/all` for `period` (passing `1d` silently falls back to `30d`). The period selected on the `用量` page applies simultaneously to `/usage` and `/usage/requests`.
- magpie has no public endpoint listing agents. `/dash/agents` is assembled locally by the bridge: `applied` from `~/.config/magpie/library.json` + models from `applied.json` + 30-day daily `by.agent` metrics + timestamp of the latest request.
- Session-level states ("Thinking / Writing code / Awaiting confirmation") are not exposed by the API. The three status pages are views you switch between by hand, not detected states; the request details on them come from the focused agent's latest call.
- Avatars use authentic agent logos (LobeHub icon set, `tools/agent_logos/*.svg`). `tools/gen_agents.py` pre-renders them at 20, 56, and 88 px into `agent_sprites.h` for 1:1 pixel rendering on screen. Logos remain the property of their respective owners.

## Latency

All numbers are measured directly on the board using serial commands (`P`, `p`, `q`, `l`), not estimates.

### Sub-second page switches

Page switching (touch press-down to full page rendered), triggered via serial command `P` simulating function key presses through the LVGL input driver:

| Metric | Previous UI | Current UI |
|---|---|---|
| Press-down to full page render | 46 ms after release (~150 ms with physical finger dwell) | **51 ms average / 57 ms worst-case**, independent of hold time |
| Repaint area on page switch | Full 480×272 screen | 480×226 page band + two function key cells |

Optimizations:

- **Press-down switches pages**: A physical tap holds 80–150 ms before release. The previous UI waited for release before rendering. Because the function key bar does not scroll, press-down is guaranteed to be a click and can trigger rendering immediately.
- **Only repaint the middle band**: The prompt bar and function key bar are globally persistent and remain unchanged during page switches. The page area itself measures only 480×226.
- **Eliminate expensive draws**: Removed `额度` quota arcs and scaled bitmaps. Replaced 110-point polyline separators (which required masked line segment drawing for each segment) with single 1 px rectangles. Render progress bars using single `■` labels instead of 20 distinct objects. Breathing animations no longer trigger invalidations when coordinates remain unchanged.
- **Rapid tap coalescing**: a page request more than 150 ms after the previous paint renders immediately; requests that land inside that window collapse into the latest target and paint once, 20 ms after the last one. Serial `b` therefore produces two paints when it starts from idle.
- **Defer data refreshes during interaction**: Data repaints are postponed while a finger touches the screen, up to a maximum of 3 seconds.

Low-level display and touch tuning history:

| Version | Full-screen repaint | Flush time (QSPI) |
|---|---|---|
| 480×20 double buffer, internal RAM | 134–157 ms | 41 ms |
| Full-screen buffer in PSRAM | 71–78 ms | 40 ms |
| + QSPI 40 MHz (exact 80 MHz APB divide-by-2; requesting 32 MHz yields ~20 MHz) | 62–71 ms | 32 ms |
| + 72 lines × 2 internal DMA RAM double buffer + async flush task (transfers band N while rendering N+1) | 34–40 ms | Overlapped with rendering |

- Number of bands matters more than buffer memory location: a 480×20 buffer forces LVGL to traverse the object tree 14 times per redraw.
- LVGL heap resides in PSRAM (the object tree is too large for internal RAM), reserving internal heap for the WiFi driver.
- Touch polling is decoupled from LVGL: the GT911 is sampled from `display_tick()` at most once every 12 ms. The LVGL read callback only consumes the latched press/release edges and never touches I²C; the display flush task does not poll touch at all. Latching is what keeps taps from being dropped during a full-screen repaint.
- Both `LV_DISP_DEF_REFR_PERIOD` and `LV_INDEV_DEF_READ_PERIOD` are set to 15 ms (default is 30 ms).

### Non-blocking serial logging

When the host stops reading the serial port (bridge halted or USB suspended), the CDC transmit ring fills up. By default, `Serial.printf` blocks the calling task, locking both the data retrieval task and the main loop.

The firmware sets `Serial.setTxTimeoutMs(0)` to drop frames instead of blocking. Periodic logging checks `availableForWrite()` first. Per-event logging is disabled by default (enabled with serial `V`).

Regression test: `tools/test_usb_disconnect.py` disconnects the serial port for 100 seconds; the board resumes communication immediately upon reconnection.

## Serial commands

While the USB bridge is running, raw serial bytes are discarded. Commands must be sent as `C` frames (`usb_bridge.py --command`, `board_shot.py --cmd`). An `@x,y` frame simulates a physical tap at coordinates `(x, y)`.

| Key | Action |
|---|---|
| `0`..`9` | Switch to page N |
| `m` / `r` | Load demo data / Return to live data and refresh immediately |
| `s` / `S` | Capture all 10 pages / Capture current screen only (preserves overlays and popups) |
| `P` | Measure page switch latency: press-down to screen render across 12 trials, reporting average and worst-case |
| `p` / `l` / `q` | Per-page full redraw time (split into flush / render) / Per-page idle redraw overhead / Main loop and refresh timing every 2 seconds |
| `A` / `Z` / `B` | Regressions: hit tests (66 checks) / Settings page (6 checks) / Agent details (6 checks), routed through the LVGL input driver |
| `b` | 4 rapid consecutive taps (verifies tap coalescing) |
| `f` / `k` / `o` | Next agent avatar / Tap current avatar / Open details sheet for top row |
| `F` / `L` `X` / `y` `Y` | Cycle `用量` period / Open, close connection panel / Open, close keyboard |
| `G` / `H` | Scroll route log up / down |
| `T` / `c` `x` / `K` | Color and touch direction test pattern / Open, close calibration screen / Clear touch calibration from NVS |
| `t` / `d` / `V` / `D` | Raw touch coordinates / Object tree dump / Toggle per-event logging / Reset settings to defaults |

`A`, `Z`, and `B` validate LVGL hit detection and navigation logic directly without querying the GT911. Physical touch behavior must be verified on hardware.

## Colors and touch direction

- Display colors use driver default `MADCTL = 0xC0` (MX|MY|RGB). If red and blue appear swapped, send serial `T` to inspect the four color blocks to identify panel channel order, endianness, or color inversion before altering assets.
- Touch coordinates map 1:1 (`ax=1 bx=0 ay=1 by=0`). An inverted calibration saved in NVS previously inverted all axes, causing taps to appear unresponsive. Send serial `K` to clear stored calibration data.

## Repository layout

```
firmware/magepie_dash/   Arduino project (ui.cpp: UI, gateway.cpp: data polling, usb_link.cpp: protocol, display_port.cpp: display and touch)
firmware/lv_conf.h       LVGL configuration; copy to Arduino libraries/ directory
tools/                   USB bridge, relay, font/sprite generators, screenshot/recording tools, regression tests
docs/                    Screenshots and demo assets for README
.github/                 CI workflows, PR templates, dependabot configuration
```

## Development

```bash
python tools/gen_fonts.py          # Rebuild font subsets (Ark Pixel monospace 10/12/24, 1bpp)
python tools/check_glyphs.py       # Verify all UI string glyphs exist in the font subsets (runs in CI)
python tools/gen_agents.py --write  # agent logos -> agent_sprites.h (needs resvg-py, pillow)
# (tools/extract_sprites.py needs real/pages/page*.png, which are not in the repo)
```

- Font subsets are extracted directly from string literals in firmware source code. Re-run `gen_fonts.py` after adding UI text. Dynamic strings from the gateway (such as account names) must be listed in its `STRINGS` array. Characters missing from Ark Pixel 12 px (e.g. "即", "热") will fail `check_glyphs.py` and must be rephrased.
- CI (runs on every PR): Builds firmware using the identical toolchain specified above, uploading the resulting `.bin` as an artifact. Also runs tool script compilation checks, USB protocol unit tests, and glyph validation. The `master` branch is protected: changes require a pull request, passing CI, and an up-to-date branch. See [CONTRIBUTING.md](CONTRIBUTING.md).

### Screenshots

Stop the USB bridge before capturing screenshots to release the serial port. While holding the serial connection, `tools/board_shot.py` answers data queries on behalf of the bridge, keeping the link active:

```bash
py -3.11 tools/board_shot.py --demo --all --out shots          # Capture all 10 pages using demo data
py -3.11 tools/board_shot.py --cmd 4 --cmd o --shot --out shots # Open Agent details sheet, then capture current screen
py -3.11 tools/board_shot.py --tap 120,258 --shot --out shots   # Simulate physical tap, then capture screen
py -3.11 tools/demo_reel.py capture && py -3.11 tools/demo_reel.py render   # Re-record demo assets for README
```

Frames travel as row bands with an offset and a checksum (`OOOOO:<hex>*SS`). If lines are missing or corrupt the tool re-dumps the whole current screen, up to four rounds, and merges the bands that pass; anything still missing is reported as an incomplete capture.

## License

[MIT](LICENSE). Agent logos remain the property of their respective owners. The Ark Pixel font is licensed under SIL OFL 1.1.