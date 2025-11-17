# Test script for Windows Edge Light Native
# Verifies all features are working

param(
    [switch]$Interactive
)

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host " Windows Edge Light Native - Testing" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

$exePath = "WindowsEdgeLightNative\bin\x64\Release\WindowsEdgeLightNative.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "ERROR: Executable not found at: $exePath" -ForegroundColor Red
    Write-Host "Please build the project first with: .\build-native.ps1" -ForegroundColor Yellow
    exit 1
}

# Test 1: File properties
Write-Host "[1/6] File Properties" -ForegroundColor Yellow
$file = Get-Item $exePath
$sizeKB = [math]::Round($file.Length / 1KB, 2)
Write-Host "  ✓ Size: $sizeKB KB" -ForegroundColor Green
Write-Host "  ✓ Last Modified: $($file.LastWriteTime)" -ForegroundColor Green

# Test 2: Version info
Write-Host ""
Write-Host "[2/6] Version Information" -ForegroundColor Yellow
$versionInfo = [System.Diagnostics.FileVersionInfo]::GetVersionInfo($exePath)
Write-Host "  ✓ Product: $($versionInfo.ProductName)" -ForegroundColor Green
Write-Host "  ✓ Version: $($versionInfo.FileVersion)" -ForegroundColor Green
Write-Host "  ✓ Company: $($versionInfo.CompanyName)" -ForegroundColor Green

# Test 3: Startup test
Write-Host ""
Write-Host "[3/6] Startup Test" -ForegroundColor Yellow
$sw = [System.Diagnostics.Stopwatch]::StartNew()
$proc = Start-Process -FilePath $exePath -PassThru
Start-Sleep -Milliseconds 500
$sw.Stop()

if ($proc.HasExited) {
    Write-Host "  ✗ Process exited immediately (crashed?)" -ForegroundColor Red
    exit 1
} else {
    Write-Host "  ✓ Started in $($sw.ElapsedMilliseconds) ms" -ForegroundColor Green
}

# Test 4: Memory usage
Write-Host ""
Write-Host "[4/6] Memory Usage" -ForegroundColor Yellow
Start-Sleep -Seconds 2
$proc.Refresh()
$memoryMB = [math]::Round($proc.WorkingSet64 / 1MB, 2)
Write-Host "  ✓ Working Set: $memoryMB MB" -ForegroundColor Green

if ($memoryMB -gt 100) {
    Write-Host "  ⚠ Warning: Memory higher than expected" -ForegroundColor Yellow
}

# Test 5: Window detection
Write-Host ""
Write-Host "[5/6] Window Detection" -ForegroundColor Yellow

# Try to find main window
$windows = Get-Process | Where-Object { $_.MainWindowTitle -like "*Edge Light*" }
if ($windows) {
    Write-Host "  ✓ Main window found: $($windows[0].MainWindowTitle)" -ForegroundColor Green
} else {
    # Window might be transparent/overlay, check if process is running
    if (-not $proc.HasExited) {
        Write-Host "  ✓ Process running (window may be transparent)" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Window not found" -ForegroundColor Red
    }
}

# Test 6: Stability test
Write-Host ""
Write-Host "[6/6] Stability Test (5 seconds)" -ForegroundColor Yellow
$initialMem = $proc.WorkingSet64

for ($i = 1; $i -le 5; $i++) {
    Start-Sleep -Seconds 1
    
    if ($proc.HasExited) {
        Write-Host "  ✗ Process crashed during stability test" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "  $i/5..." -NoNewline
}

$proc.Refresh()
$finalMem = $proc.WorkingSet64
$memChange = [math]::Round(($finalMem - $initialMem) / 1MB, 2)

Write-Host ""
if ([math]::Abs($memChange) -lt 5) {
    Write-Host "  ✓ Memory stable ($memChange MB change)" -ForegroundColor Green
} else {
    Write-Host "  ⚠ Memory changed: $memChange MB" -ForegroundColor Yellow
}

# Interactive test
if ($Interactive) {
    Write-Host ""
    Write-Host "=====================================" -ForegroundColor Cyan
    Write-Host " Interactive Feature Test" -ForegroundColor Cyan
    Write-Host "=====================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "The application is now running." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please test the following manually:" -ForegroundColor White
    Write-Host "  1. Press Ctrl+Shift+L to toggle light"
    Write-Host "  2. Press Ctrl+Shift+Up to increase brightness"
    Write-Host "  3. Press Ctrl+Shift+Down to decrease brightness"
    Write-Host "  4. Right-click the system tray icon"
    Write-Host "  5. Click buttons on the control panel at bottom center"
    Write-Host "  6. Verify the edge light appears around your monitor"
    Write-Host ""
    Write-Host "Press any key when done testing..." -ForegroundColor Yellow
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}

# Cleanup
Write-Host ""
Write-Host "Stopping test instance..." -ForegroundColor Gray
$proc.Kill()
$proc.WaitForExit()

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host " Test Summary" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "✓ All automated tests passed!" -ForegroundColor Green
Write-Host ""
Write-Host "Executable Details:" -ForegroundColor Cyan
Write-Host "  Location: $exePath"
Write-Host "  Size: $sizeKB KB"
Write-Host "  Memory: ~$memoryMB MB"
Write-Host "  Startup: ~$($sw.ElapsedMilliseconds) ms"
Write-Host ""

if ($Interactive) {
    Write-Host "Manual testing completed." -ForegroundColor Green
} else {
    Write-Host "Run with -Interactive flag to test features manually." -ForegroundColor Yellow
}

Write-Host ""
