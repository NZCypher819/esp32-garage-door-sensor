# WiFi Setup Script for ESP32 Garage Door Project
# This script helps you securely configure WiFi credentials

Write-Host "=== ESP32 WiFi Configuration Setup ===" -ForegroundColor Green
Write-Host ""

# Check if secrets.h exists
$secretsFile = "include/secrets.h"
if (Test-Path $secretsFile) {
    Write-Host "✅ secrets.h file found!" -ForegroundColor Green
} else {
    Write-Host "❌ secrets.h file not found!" -ForegroundColor Red
    Write-Host "Creating secrets.h from template..." -ForegroundColor Yellow
    Copy-Item "include/secrets.h.template" $secretsFile
}

Write-Host ""
Write-Host "Current WiFi Configuration:" -ForegroundColor Cyan
Write-Host "SSID: U64A 2.4" -ForegroundColor White
Write-Host ""

# Prompt for password
$password = Read-Host "Enter your WiFi password" -AsSecureString
$plainPassword = [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($password))

if ($plainPassword -eq "") {
    Write-Host "❌ No password entered. Please edit include/secrets.h manually." -ForegroundColor Red
    exit 1
}

# Update secrets.h file
$content = Get-Content $secretsFile
$newContent = $content -replace 'YOUR_ACTUAL_PASSWORD_HERE', $plainPassword
$newContent | Set-Content $secretsFile

Write-Host ""
Write-Host "✅ WiFi credentials configured successfully!" -ForegroundColor Green
Write-Host "🔒 Password is stored securely in include/secrets.h" -ForegroundColor Yellow
Write-Host "⚠️  This file is excluded from Git commits" -ForegroundColor Yellow
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Build and upload your project" -ForegroundColor White
Write-Host "2. Open serial monitor to see WiFi connection status" -ForegroundColor White
Write-Host "3. Your ESP32 will connect to 'U64A 2.4' automatically" -ForegroundColor White

# Clean up
$plainPassword = $null
[System.GC]::Collect()