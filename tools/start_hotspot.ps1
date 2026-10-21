# Start a 2.4 GHz-friendly mobile hotspot so the ESP32 can join the same LAN
# as this PC (the campus WiFi is 5 GHz + 802.1X, which the S3 cannot use).
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Runtime.WindowsRuntime
$asTaskGeneric = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncOperation`1'
})[0]

function Await($op, $resultType) {
    $task = $asTaskGeneric.MakeGenericMethod($resultType).Invoke($null, @($op))
    $task.Wait(-1) | Out-Null
    $task.Result
}

$asTaskAction = ([System.WindowsRuntimeSystemExtensions].GetMethods() | Where-Object {
    $_.Name -eq 'AsTask' -and $_.GetParameters().Count -eq 1 -and
    $_.GetParameters()[0].ParameterType.Name -eq 'IAsyncAction'
})[0]

function AwaitAction($op) {
    $task = $asTaskAction.Invoke($null, @($op))
    $task.Wait(-1) | Out-Null
}

[Windows.Networking.Connectivity.NetworkInformation, Windows.Networking.Connectivity, ContentType = WindowsRuntime] | Out-Null
[Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager, Windows.Networking.NetworkOperators, ContentType = WindowsRuntime] | Out-Null

$cp = [Windows.Networking.Connectivity.NetworkInformation]::GetInternetConnectionProfile()
if (-not $cp) { Write-Output 'no internet profile'; exit 1 }
Write-Output ("internet profile: " + $cp.ProfileName)

$mgr = [Windows.Networking.NetworkOperators.NetworkOperatorTetheringManager]::CreateFromConnectionProfile($cp)
$cfg = $mgr.GetCurrentAccessPointConfiguration()
$cfg.Ssid = 'magpie-dash-net'
$cfg.Passphrase = 'magpie12345'
AwaitAction ($mgr.ConfigureAccessPointAsync($cfg))
Start-Sleep -Seconds 1

$res = Await ($mgr.StartTetheringAsync()) ([Windows.Networking.NetworkOperators.NetworkOperatorTetheringOperationResult])
Write-Output ("tethering: " + $res.Status + " " + $res.AdditionalErrorMessage)
Start-Sleep -Seconds 3

$ip = (Get-NetIPAddress -AddressFamily IPv4 | Where-Object {
    $_.InterfaceAlias -match 'Local Area Connection|WLAN|Wi-Fi' -and $_.IPAddress -like '192.168.137.*'
})
if ($ip) { Write-Output ("hotspot gateway ip: " + $ip.IPAddress) } else { Write-Output 'hotspot ip not found yet' }
Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -like '192.168.*' } | Select-Object IPAddress, InterfaceAlias | Format-Table -AutoSize | Out-String -Width 100
