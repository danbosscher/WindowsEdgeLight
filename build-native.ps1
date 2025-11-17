# Build script for Windows Edge Light Native
# Builds both x64 and ARM64 versions

param(
    [string]$Configuration = "Release"
)

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host " Windows Edge Light - Native Build" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Find Visual Studio Build Tools
$vsPath = & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath 2>$null

if (-not $vsPath) {
    $vsPath = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools"
    if (-not (Test-Path $vsPath)) {
        Write-Host "ERROR: Visual Studio Build Tools not found!" -ForegroundColor Red
        Write-Host ""
        Write-Host "Please install with:" -ForegroundColor Yellow
        Write-Host "  winget install Microsoft.VisualStudio.2022.BuildTools --silent --override `"--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended`""
        exit 1
    }
}

$vcvarsall = Join-Path $vsPath "VC\Auxiliary\Build\vcvarsall.bat"

if (-not (Test-Path $vcvarsall)) {
    Write-Host "ERROR: vcvarsall.bat not found at: $vcvarsall" -ForegroundColor Red
    exit 1
}

Write-Host "Using Visual Studio at: $vsPath" -ForegroundColor Green
Write-Host ""

# Build x64
Write-Host "Building x64 version..." -ForegroundColor Yellow
Push-Location WindowsEdgeLightNative
$buildCmd = @"
call "$vcvarsall" x64 >nul 2>&1 && msbuild WindowsEdgeLightNative.vcxproj /p:Configuration=$Configuration /p:Platform=x64 /v:minimal /nologo
"@
cmd /c $buildCmd
$x64Success = $LASTEXITCODE -eq 0
Pop-Location

Write-Host ""

# Build ARM64
Write-Host "Building ARM64 version..." -ForegroundColor Yellow
Push-Location WindowsEdgeLightNative
$buildCmd = @"
call "$vcvarsall" x64_arm64 >nul 2>&1 && msbuild WindowsEdgeLightNative.vcxproj /p:Configuration=$Configuration /p:Platform=ARM64 /v:minimal /nologo
"@
cmd /c $buildCmd
$arm64Success = $LASTEXITCODE -eq 0
Pop-Location

Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host " Build Summary" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# Check outputs
$outputs = @()

$x64Exe = "WindowsEdgeLightNative\bin\x64\$Configuration\WindowsEdgeLightNative.exe"
if (Test-Path $x64Exe) {
    $size = (Get-Item $x64Exe).Length
    $sizeKB = [math]::Round($size / 1KB, 2)
    $outputs += [PSCustomObject]@{
        Platform = "x64"
        Status = "✓ Success"
        Size = "$sizeKB KB"
        Path = $x64Exe
    }
} else {
    $outputs += [PSCustomObject]@{
        Platform = "x64"
        Status = "✗ Failed"
        Size = "-"
        Path = "-"
    }
}

$arm64Exe = "WindowsEdgeLightNative\bin\ARM64\$Configuration\WindowsEdgeLightNative.exe"
if (Test-Path $arm64Exe) {
    $size = (Get-Item $arm64Exe).Length
    $sizeKB = [math]::Round($size / 1KB, 2)
    $outputs += [PSCustomObject]@{
        Platform = "ARM64"
        Status = "✓ Success"
        Size = "$sizeKB KB"
        Path = $arm64Exe
    }
} else {
    $outputs += [PSCustomObject]@{
        Platform = "ARM64"
        Status = "✗ Failed"
        Size = "-"
        Path = "-"
    }
}

$outputs | Format-Table -AutoSize

# Copy to publish folder
$publishDir = "publish-native"
if (-not (Test-Path $publishDir)) {
    New-Item -ItemType Directory -Path $publishDir | Out-Null
}

$version = "2.0"
$copied = @()

if (Test-Path $x64Exe) {
    $dest = "$publishDir\WindowsEdgeLightNative-v$version-win-x64.exe"
    Copy-Item $x64Exe $dest -Force
    $copied += $dest
}

if (Test-Path $arm64Exe) {
    $dest = "$publishDir\WindowsEdgeLightNative-v$version-win-arm64.exe"
    Copy-Item $arm64Exe $dest -Force
    $copied += $dest
}

if ($copied.Count -gt 0) {
    Write-Host ""
    Write-Host "Published to:" -ForegroundColor Green
    foreach ($file in $copied) {
        Write-Host "  $file" -ForegroundColor Cyan
    }
}

Write-Host ""

# Compare with WPF version if it exists
$wpfExe = "WindowsEdgeLight\bin\Release\net10.0-windows\win-x64\publish\WindowsEdgeLight.exe"
if ((Test-Path $x64Exe) -and (Test-Path $wpfExe)) {
    $nativeSize = (Get-Item $x64Exe).Length
    $wpfSize = (Get-Item $wpfExe).Length
    $ratio = [math]::Round($wpfSize / $nativeSize, 1)
    $reduction = [math]::Round((1 - $nativeSize / $wpfSize) * 100, 1)
    
    Write-Host "Size Comparison (x64):" -ForegroundColor Cyan
    Write-Host "  WPF Version:    $([math]::Round($wpfSize / 1MB, 2)) MB"
    Write-Host "  Native Version: $([math]::Round($nativeSize / 1KB, 2)) KB"
    Write-Host "  Reduction:      $reduction% ($ratio`x smaller)" -ForegroundColor Green
    Write-Host ""
}

Write-Host "Done!" -ForegroundColor Green
