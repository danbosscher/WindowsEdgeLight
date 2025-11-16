# Measure WindowsEdgeLight Optimization Results
# This script helps measure the impact of optimizations

param(
    [string]$ExePath = "",
    [switch]$MonitorMemory = $false,
    [int]$MonitorDuration = 30
)

Write-Host "WindowsEdgeLight Optimization Measurement Tool" -ForegroundColor Cyan
Write-Host "=" * 60 -ForegroundColor Cyan
Write-Host ""

# Baseline values
$baselineSize = 72
$baselineMemory = 255.7

# Find executable if not specified
if ([string]::IsNullOrEmpty($ExePath)) {
    Write-Host "Searching for WindowsEdgeLight.exe..." -ForegroundColor Yellow
    
    # Try build output directories
    $searchPaths = @(
        "WindowsEdgeLight\bin\Release\net10.0-windows\win-x64\publish\WindowsEdgeLight.exe",
        "publish\WindowsEdgeLight-v*-win-x64.exe"
    )
    
    foreach ($path in $searchPaths) {
        $resolved = Resolve-Path $path -ErrorAction SilentlyContinue
        if ($resolved) {
            if ($resolved.Count -gt 1) {
                $ExePath = $resolved[-1].Path  # Get the latest
            } else {
                $ExePath = $resolved.Path
            }
            break
        }
    }
    
    if ([string]::IsNullOrEmpty($ExePath)) {
        Write-Host "ERROR: Could not find WindowsEdgeLight.exe" -ForegroundColor Red
        Write-Host "Please build the project first using: .\build.ps1" -ForegroundColor Yellow
        exit 1
    }
}

# Check if file exists
if (-not (Test-Path $ExePath)) {
    Write-Host "ERROR: File not found: $ExePath" -ForegroundColor Red
    exit 1
}

Write-Host "Found executable: $ExePath" -ForegroundColor Green
Write-Host ""

# Measure executable size
Write-Host "Measuring Executable Size" -ForegroundColor Cyan
Write-Host "-" * 60
$fileSize = (Get-Item $ExePath).Length
$fileSizeMB = [math]::Round($fileSize / 1MB, 2)
$sizeReduction = [math]::Round((($baselineSize - $fileSizeMB) / $baselineSize) * 100, 1)

Write-Host "Baseline:  $baselineSize MB" -ForegroundColor White
Write-Host "Optimized: $fileSizeMB MB" -ForegroundColor Green
if ($sizeReduction -gt 0) {
    Write-Host "Reduction: $sizeReduction% ($([math]::Round($baselineSize - $fileSizeMB, 2)) MB saved)" -ForegroundColor Green
} else {
    Write-Host "Change:    $([math]::Abs($sizeReduction))% increase" -ForegroundColor Yellow
}
Write-Host ""

# Memory monitoring
if ($MonitorMemory) {
    Write-Host "Monitoring Memory Usage" -ForegroundColor Cyan
    Write-Host "-" * 60
    Write-Host "Launching application..." -ForegroundColor Yellow
    
    try {
        # Launch the application
        $process = Start-Process -FilePath $ExePath -PassThru -WindowStyle Normal
        
        if (-not $process) {
            Write-Host "ERROR: Failed to launch application" -ForegroundColor Red
            exit 1
        }
        
        Write-Host "Application launched (PID: $($process.Id))" -ForegroundColor Green
        Write-Host "Monitoring memory for $MonitorDuration seconds..." -ForegroundColor Yellow
        Write-Host ""
        
        # Wait a bit for the app to fully initialize
        Start-Sleep -Seconds 3
        
        # Collect memory samples
        $samples = @()
        $sampleCount = [math]::Min($MonitorDuration, 10)  # Sample every 1-3 seconds
        $interval = [math]::Max(1, [math]::Floor($MonitorDuration / $sampleCount))
        
        for ($i = 0; $i -lt $sampleCount; $i++) {
            try {
                $proc = Get-Process -Id $process.Id -ErrorAction Stop
                $memoryMB = [math]::Round($proc.WorkingSet64 / 1MB, 2)
                $samples += $memoryMB
                
                $timestamp = Get-Date -Format "HH:mm:ss"
                Write-Host "[$timestamp] Memory: $memoryMB MB" -ForegroundColor White
                
                if ($i -lt $sampleCount - 1) {
                    Start-Sleep -Seconds $interval
                }
            } catch {
                Write-Host "WARNING: Process may have exited" -ForegroundColor Yellow
                break
            }
        }
        
        Write-Host ""
        
        # Calculate statistics
        if ($samples.Count -gt 0) {
            $avgMemory = [math]::Round(($samples | Measure-Object -Average).Average, 2)
            $minMemory = [math]::Round(($samples | Measure-Object -Minimum).Minimum, 2)
            $maxMemory = [math]::Round(($samples | Measure-Object -Maximum).Maximum, 2)
            
            Write-Host "Memory Statistics:" -ForegroundColor Cyan
            Write-Host "  Initial:  $($samples[0]) MB" -ForegroundColor White
            Write-Host "  Average:  $avgMemory MB" -ForegroundColor White
            Write-Host "  Minimum:  $minMemory MB" -ForegroundColor White
            Write-Host "  Maximum:  $maxMemory MB" -ForegroundColor White
            Write-Host ""
            
            $memReduction = [math]::Round((($baselineMemory - $avgMemory) / $baselineMemory) * 100, 1)
            Write-Host "Baseline:  $baselineMemory MB" -ForegroundColor White
            Write-Host "Optimized: $avgMemory MB (average)" -ForegroundColor Green
            if ($memReduction -gt 0) {
                Write-Host "Reduction: $memReduction% ($([math]::Round($baselineMemory - $avgMemory, 2)) MB saved)" -ForegroundColor Green
            } else {
                Write-Host "Change:    $([math]::Abs($memReduction))% increase" -ForegroundColor Yellow
            }
            Write-Host ""
        }
        
        # Ask to close the app
        Write-Host "Press any key to close the application..." -ForegroundColor Yellow
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        
        # Close the application
        try {
            $process.CloseMainWindow() | Out-Null
            Start-Sleep -Seconds 2
            if (-not $process.HasExited) {
                $process.Kill()
            }
            Write-Host "Application closed" -ForegroundColor Green
        } catch {
            Write-Host "Note: Application may have already closed" -ForegroundColor Yellow
        }
        
    } catch {
        Write-Host "ERROR: $($_.Exception.Message)" -ForegroundColor Red
    }
}

# Summary
Write-Host ""
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "=" * 60
Write-Host "Executable Size: $fileSizeMB MB (was $baselineSize MB, $sizeReduction% reduction)" -ForegroundColor White
if ($MonitorMemory -and $samples.Count -gt 0) {
    Write-Host "Memory Usage:    $avgMemory MB (was $baselineMemory MB, $memReduction% reduction)" -ForegroundColor White
} else {
    Write-Host "Memory Usage:    Not measured (use -MonitorMemory to measure)" -ForegroundColor Gray
}
Write-Host ""

# Recommendations
if ($fileSizeMB -ge 65) {
    Write-Host "⚠️  Executable size is still quite large." -ForegroundColor Yellow
    Write-Host "    Consider checking if optimizations were applied correctly." -ForegroundColor Yellow
} elseif ($fileSizeMB -ge 50) {
    Write-Host "✓  Good size reduction achieved." -ForegroundColor Green
    Write-Host "   Additional optimizations may be possible (see OPTIMIZATION_NOTES.md)." -ForegroundColor Yellow
} else {
    Write-Host "✓✓ Excellent size reduction!" -ForegroundColor Green
}

Write-Host ""
Write-Host "For complete testing, see TESTING_CHECKLIST.md" -ForegroundColor Cyan
