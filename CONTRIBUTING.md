# Contributing

`master` is protected: changes land through a pull request with green CI.

## CI (`.github/workflows/ci.yml`)

| job | what it checks |
|---|---|
| `host tools` | every script in `tools/` byte-compiles, the USB protocol unit tests pass, and every string the firmware draws has a glyph in the bundled pixel font (`tools/check_glyphs.py`) |
| `firmware (ESP32-S3)` | the sketch compiles with the exact toolchain of the bench build (esp32 core 3.3.12, LVGL 8.3.7, ArduinoJson 7.4.3, GFX Library for Arduino 1.6.9, `firmware/lv_conf.h`); the `.bin` files are kept as an artifact |

CI cannot touch the hardware. For UI changes also run the on-device suites (serial `A`, `Z`, `B`) and attach screenshots; the PR template lists the commands.

## Rules on `master`

- no direct pushes, no force pushes, no deletion
- a pull request is required; both CI jobs must pass and the branch must be up to date
- review conversations must be resolved before merging
- repository admins may bypass in an emergency

## Local build

```bash
bash flash.sh          # compile only
bash flash.sh flash    # compile + upload + 10 s of serial log
```

Toolchain paths are set at the top of `flash.sh`; `firmware/lv_conf.h` goes next to the `lvgl` folder in your Arduino `libraries` directory.
