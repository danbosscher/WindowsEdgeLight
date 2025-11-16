# WindowsEdgeLight Optimization Notes

This document details the optimizations applied to reduce the executable size and memory footprint of WindowsEdgeLight.

## Original State
- Executable Size: ~72MB
- Memory Usage: ~255.7MB RAM
- Configuration: Self-contained .NET 10 WPF app with no trimming

## Optimizations Applied

### 1. IL Trimming (`PublishTrimmed=true`)
- **Impact**: Removes unused code from .NET assemblies
- **Mode**: `partial` - Compatible with WPF/WinForms reflection requirements
- **Configuration**: Added `TrimmerRootAssembly` entries for core WPF assemblies to preserve essential functionality
- **Expected Reduction**: 15-25% of executable size

### 2. ReadyToRun Compilation (`PublishReadyToRun=true`)
- **Impact**: Pre-compiles IL to native code for faster startup and better memory efficiency
- **Trade-off**: Slightly larger executable but significantly lower runtime memory and faster startup
- **Expected Impact**: Reduced JIT memory overhead (~10-15% memory reduction)

### 3. Globalization Optimization (`InvariantGlobalization=true`)
- **Impact**: Removes culture-specific data and ICU libraries
- **Trade-off**: Only invariant culture available (English-like formatting)
- **Expected Reduction**: ~10-15MB from executable size

### 4. Runtime Feature Disabling
Disabled unused .NET runtime features:
- `EventSourceSupport=false` - No EventSource tracing
- `HttpActivityPropagationSupport=false` - No HTTP activity correlation
- `MetadataUpdaterSupport=false` - No hot reload support
- `EnableUnsafeBinaryFormatterSerialization=false` - Security improvement
- `EnableUnsafeUTF7Encoding=false` - Security improvement
- **Expected Impact**: Reduced runtime overhead (~5-10MB)

### 5. Garbage Collection Optimization
Runtime configuration for lightweight desktop app:
- Workstation GC mode instead of Server GC
- Concurrent GC enabled for better responsiveness
- RetainVM disabled to release memory more aggressively
- Reduced thread pool size (1 min, 32 max threads)
- **Expected Impact**: 20-30% memory usage reduction

### 6. Win32 API Usage for Monitor Detection
- Replaced `System.Windows.Forms.Screen` with direct Win32 `EnumDisplayMonitors` API
- Created lightweight `MonitorInfo` class
- Reduced Windows Forms dependency footprint (now only used for NotifyIcon)
- **Expected Impact**: Better trimming of Windows Forms assemblies

### 7. IL Compiler Optimizations
- `IlcOptimizationPreference=Speed` - Optimize for execution speed
- `IlcGenerateStackTraceData=false` - Reduce debug overhead
- `UseSystemResourceKeys=true` - Use system resource keys
- **Expected Impact**: Minor size and memory improvements

### 8. Linker Substitutions
- Added `ILLink.Substitutions.xml` to stub out unused diagnostic and expression compiler code
- **Expected Impact**: Additional trimming of System.Linq.Expressions and diagnostics

## Expected Combined Impact

### Executable Size
- **Before**: ~72MB
- **Expected After**: ~40-50MB (30-45% reduction)
- **Best Case**: ~35-40MB with aggressive trimming

### Memory Usage
- **Before**: ~255.7MB
- **Expected After**: ~150-180MB (30-40% reduction)
- **Best Case**: ~120-150MB with optimal GC settings

## Compatibility Notes

1. **Culture/Localization**: App now only supports invariant culture (English-like formatting)
2. **Reflection**: Partial trimming mode maintains compatibility with WPF's XAML reflection requirements
3. **Windows Only**: Optimizations are Windows-specific (no impact on portability)
4. **Hot Reload**: Disabled in production builds (development workflow unaffected)

## Testing Requirements

1. **Functional Testing**:
   - Verify edge light displays correctly on all monitors
   - Test monitor switching functionality
   - Verify hotkeys work globally
   - Test NotifyIcon context menu
   - Verify brightness controls

2. **Performance Testing**:
   - Measure actual executable size after publish
   - Monitor memory usage with Task Manager/Performance Monitor
   - Test startup time
   - Verify no memory leaks during extended use

3. **Compatibility Testing**:
   - Test on different Windows versions (10, 11)
   - Test with different DPI settings
   - Test multi-monitor configurations

## Build Instructions

To build with optimizations:
```powershell
dotnet publish WindowsEdgeLight\WindowsEdgeLight.csproj `
    -c Release `
    -r win-x64 `
    /p:DebugType=None `
    /p:DebugSymbols=false `
    --self-contained
```

Or use the provided build script:
```powershell
.\build.ps1
```

## Further Optimization Opportunities

If more reduction is needed:
1. Consider framework-dependent deployment (requires .NET runtime on target machine, but reduces to ~1-2MB)
2. Make Updatum dependency optional with conditional compilation
3. Use Native AOT (requires significant code changes for WPF compatibility)
4. Consider alternative UI frameworks (e.g., WinUI 3) for better trimming support
