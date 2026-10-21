#!/usr/bin/env bash
# No argument: COMPILE ONLY. Explicit 'flash': compile + upload + serial dump.
set -euo pipefail
if [[ $# -gt 1 || ( $# -eq 1 && "$1" != "flash" ) ]]; then
  printf 'Usage: bash flash.sh [flash]\n' >&2
  exit 2
fi
export ARDUINO_DIRECTORIES_DATA="D:/esp32/tools/arduino15"
export ARDUINO_DIRECTORIES_USER="D:/esp32/tools/Arduino"
CLI="/d/esp32/tools/arduino-cli.exe"
FQBN="esp32:esp32:esp32s3:USBMode=hwcdc,CDCOnBoot=cdc,PSRAM=opi,FlashSize=4M,PartitionScheme=huge_app,CPUFreq=240,FlashMode=qio,UploadSpeed=921600,DebugLevel=none"
cd /d/esp32/magepie-esp32
mkdir -p build_logs
# Keep logs in the project, never in the Windows/system temporary directory.
# Limit parallel compilers: hundreds of simultaneous xtensa g++ processes have
# failed on this host with 0xc0000142 (DLL init failure / desktop heap exhaustion).
JOBS="${JOBS:-4}"
"$CLI" compile --jobs "$JOBS" --fqbn "$FQBN" --build-path D:/esp32/build --output-dir D:/esp32/dist firmware/magepie_dash 2>&1 | tee build_logs/build.log
if [[ "${1:-}" == "flash" ]]; then
  # The bridge owns COM9; hand the port over for the upload and give it back.
  TASK=MagpieDashUsbBridge
  had_task=$(powershell -NoProfile -Command "(Get-ScheduledTask -TaskName $TASK -ErrorAction SilentlyContinue).State" 2>/dev/null | tr -d '\r')
  if [[ "$had_task" == "Running" ]]; then
    powershell -NoProfile -Command "Stop-ScheduledTask -TaskName $TASK" >/dev/null 2>&1 || true
    sleep 2
  fi
  "$CLI" upload -p COM9 --fqbn "$FQBN" --input-dir D:/esp32/dist firmware/magepie_dash 2>&1 | tee build_logs/upload.log
  printf 'flashed\n'
  # Dump BEFORE handing the port back, otherwise the restarted bridge owns COM9.
  py -3.11 "$(dirname "$0")/tools/serial_dump.py" 10
  if [[ "$had_task" == "Running" ]]; then
    powershell -NoProfile -Command "Start-ScheduledTask -TaskName $TASK" >/dev/null 2>&1 || true
  fi
else
  printf 'Compile only: upload NOT requested. Binaries: D:/esp32/dist\n'
fi
