# Build script for Windows Edge Light Native
param(
    [string]$Configuration = "Release",
    [string]$Platform = "x64"
)

Write-Host "Building Windows Edge Light Native - $Configuration $Platform" -ForegroundColor Cyan

# Check for build tools
$hasCMake = Get-Command cmake -ErrorAction SilentlyContinue
$hasNinja = Get-Command ninja -ErrorAction SilentlyContinue

if (-not $hasCMake) {
    Write-Host "ERROR: CMake not found. Please install CMake or Visual Studio Build Tools." -ForegroundColor Red
    Write-Host ""
    Write-Host "Options to install:" -ForegroundColor Yellow
    Write-Host "1. Install Visual Studio 2022 with C++ Desktop Development workload"
    Write-Host "2. Install Visual Studio Build Tools: https://aka.ms/vs/17/release/vs_BuildTools.exe"
    Write-Host "3. Install CMake: winget install Kitware.CMake"
    Write-Host ""
    Write-Host "If you have Visual Studio installed, run this from 'Developer PowerShell for VS 2022'"
    exit 1
}

# Create build directory
$buildDir = "build_$Platform"
if (Test-Path $buildDir) {
    Remove-Item $buildDir -Recurse -Force
}
New-Item -ItemType Directory -Path $buildDir | Out-Null

Push-Location $buildDir

try {
    Write-Host "Configuring project..." -ForegroundColor Yellow
    
    $generator = "Ninja"
    if (-not $hasNinja) {
        $generator = "NMake Makefiles"
    }
    
    $cmakeArgs = @(
        ".."
        "-G", $generator
        "-DCMAKE_BUILD_TYPE=$Configuration"
    )
    
    if ($Platform -eq "ARM64") {
        $cmakeArgs += "-A", "ARM64"
    }
    
    & cmake @cmakeArgs
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }
    
    Write-Host "Building..." -ForegroundColor Yellow
    & cmake --build . --config $Configuration
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    
    Write-Host ""
    Write-Host "Build successful!" -ForegroundColor Green
    
    # Find the exe
    $exe = Get-ChildItem -Path "." -Filter "WindowsEdgeLightNative.exe" -Recurse | Select-Object -First 1
    if ($exe) {
        $size = [math]::Round($exe.Length / 1KB, 2)
        Write-Host "Executable: $($exe.FullName)" -ForegroundColor Cyan
        Write-Host "Size: $size KB" -ForegroundColor Cyan
    }
} finally {
    Pop-Location
}
