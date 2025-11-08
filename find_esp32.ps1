# Script to find ESP32 on network
Write-Host "Scanning for ESP32 on local network..."

# Get local network range
$networkInfo = Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.PrefixOrigin -eq "Dhcp" }
$localIP = $networkInfo.IPAddress
$networkBase = ($localIP -split '\.')[0..2] -join '.'

Write-Host "Local IP: $localIP"
Write-Host "Scanning network: $networkBase.1-254"

# Function to test if ESP32 web server is running on an IP
function Test-ESP32WebServer($ip) {
    try {
        $response = Invoke-WebRequest -Uri "http://$ip" -TimeoutSec 2 -ErrorAction SilentlyContinue
        if ($response.StatusCode -eq 200 -and $response.Content -like "*ESP32*") {
            return $true
        }
    } catch {
        # Ignore errors
    }
    return $false
}

# Scan common IP ranges
$found = $false
for ($i = 100; $i -le 199; $i++) {
    $testIP = "$networkBase.$i"
    if (Test-ESP32WebServer $testIP) {
        Write-Host "Found ESP32 at: http://$testIP" -ForegroundColor Green
        Write-Host "Web Dashboard: http://$testIP/" -ForegroundColor Yellow
        Write-Host "API Status: http://$testIP/api/status" -ForegroundColor Yellow
        $found = $true
        break
    }
    if ($i % 10 -eq 0) {
        Write-Host "Checked up to $testIP..." -ForegroundColor Gray
    }
}

if (-not $found) {
    Write-Host "ESP32 not found. Try these steps:" -ForegroundColor Red
    Write-Host "1. Make sure your WiFi network 'U64A 2.4' is available"
    Write-Host "2. Check if ESP32 is connected (LED should be solid on)"
    Write-Host "3. Try connecting a serial monitor to see debug output"
}