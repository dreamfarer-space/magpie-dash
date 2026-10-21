#!/usr/bin/env bash
# Build (and optionally flash) the magpie dash firmware.
#   ./build.sh          -> compile only
#   ./build.sh flash    -> compile + upload + serial monitor
set -e

export ARDUINO_DIRECTORIES_DATA="D:/esp32/tools/arduino15"
export ARDUINO_DIRECTORIES_USER="D:/esp32/tools/Arduino"
CLI="/d/esp32/tools/arduino-cli.exe"
PORT="${PORT:-COM9}"
FQBN="esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,PSRAM=opi,FlashSize=4M,PartitionScheme=huge_app,CPUFreq=240,FlashMode=qio,UploadSpeed=921600,DebugLevel=none"

cd "$(dirname "$0")/firmware"

"$CLI" compile --fqbn "$FQBN" --warnings default \
  --build-path D:/esp32/build --output-dir D:/esp32/dist magepie_dash

if [ "$1" = "flash" ]; then
  "$CLI" upload -p "$PORT" --fqbn "$FQBN" --input-dir D:/esp32/dist magepie_dash
  if [ "$2" = "mon" ]; then
    "$CLI" monitor -p "$PORT" -c baudrate=115200 --config baudrate=115200
  fi
fi
