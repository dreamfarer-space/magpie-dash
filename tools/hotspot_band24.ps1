# Force the mobile hotspot to 2.4 GHz (the ESP32-S3 has no 5 GHz radio).
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Runtime.WindowsRuntime

$asTaskAction = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncAction'
})[0]
$asTaskOp = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
})[0]

function AwaitAction($op) { $t = $asTaskAction.Invoke($null, @($op)); $t.Wait(-1) | Out-Null }
function Await($op, $t2) { $t = $asTaskOp.MakeGenericMethod($t2).Invoke($null, @($op)); $t.Wait(-1) | Out-Null; $t.Result }

[Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime] | Out-Null
[Windows.Networking.NetworkOperators.NetworkOperatorTetheringAccessPointConfiguration, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime] | Out-Null

$cp = [Windows.Networking.Connectivity.NetworkInformation]::GetInternetConnectionProfile()
$mgr = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager]::CreateFromConnectionProfile($cp)
$cfg = $mgr.GetCurrentAccessPointConfiguration()
Write-Output ("current ssid=" + $cfg.Ssid + " band=" + $cfg.Band)
$cfg.Band = [Windows.Networking.NetworkOperators.TetheringWiFiBand]::TwoPointFourGigahertz
AwaitAction ($mgr.ConfigureAccessPointAsync($cfg))
Start-Sleep -Seconds 2
$cfg2 = $mgr.GetCurrentAccessPointConfiguration()
Write-Output ("new ssid=" + $cfg2.Ssid + " band=" + $cfg2.Band)
$state = $mgr.TetheringOperability
Write-Output ("operability=" + $state)
