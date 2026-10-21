# Registers (or removes) the logon task that keeps the magpie USB bridge running.
# Runs the bridge with pythonw so no console window stays open; logs go to
# build_logs/usb_bridge.log and build_logs/usb_bridge_status.json.
param(
    [ValidateSet('install','remove','status')]
    [string]$Action = 'install',
    [string]$TaskName = 'MagpieDashUsbBridge',
    [string]$PythonW = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$script = Join-Path $root 'tools\usb_bridge.py'
if (-not $PythonW) {
    # Default: the system 3.11 install that carries pyserial (a bare 'python' may be a venv).
    $PythonW = Join-Path $env:LOCALAPPDATA 'Programs\Python\Python311\pythonw.exe'
}
$pythonw = $PythonW
if (-not (Test-Path $pythonw)) { throw "pythonw.exe not found next to python: $pythonw" }

switch ($Action) {
    'install' {
        $taskAction = New-ScheduledTaskAction -Execute $pythonw -Argument "`"$script`" --port COM9" -WorkingDirectory $root
        $trigger = New-ScheduledTaskTrigger -AtLogOn
        $settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries `
            -ExecutionTimeLimit ([TimeSpan]::Zero) -RestartCount 3 -RestartInterval ([TimeSpan]::FromMinutes(1)) `
            -MultipleInstances IgnoreNew
        Register-ScheduledTask -TaskName $TaskName -Action $taskAction -Trigger $trigger -Settings $settings `
            -Description 'Feeds magpie gateway data to the ESP32 dashboard over USB serial (read-only, no network listener).' -Force | Out-Null
        Write-Output "installed: $TaskName -> $pythonw $script"
    }
    'remove' {
        Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
        Write-Output "removed: $TaskName"
    }
    'status' {
        $t = Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue
        if (-not $t) { Write-Output 'not installed'; return }
        $i = Get-ScheduledTaskInfo -TaskName $TaskName
        Write-Output ("state={0} last={1} result={2}" -f $t.State, $i.LastRunTime, $i.LastTaskResult)
    }
}
