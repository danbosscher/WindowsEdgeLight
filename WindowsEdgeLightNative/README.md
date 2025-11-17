# Windows Edge Light - Ultra-Minimal Native Edition

**The tiniest Windows edge lighting overlay you'll ever find!**

A pure Win32 + GDI implementation that's 680x smaller than the original WPF version and uses 150x less memory. No .NET, no DirectX, no bloat - just raw Windows API magic.

![Edge Light Demo](https://via.placeholder.com/800x400/000000/FFFFFF?text=White+Glowing+Frame+Around+Screen)

## Why This Exists

The original WPF version is 72MB and uses 300MB of RAM just to draw a white rectangle. This version is **106KB** and uses **2MB of RAM**. That's it. That's the whole pitch.

## Performance

| Metric | WPF Original | This Version | Improvement |
|--------|-------------|--------------|-------------|
| **File Size** | 72,000 KB | **106 KB** | 680x smaller |
| **Memory (Private)** | 300,000 KB | **~2,000 KB** | 150x less |
| **Dependencies** | .NET 10 Runtime + WPF | Windows API only | None needed |
| **Startup Time** | ~800ms | **<50ms** | 16x faster |
| **CPU Usage (idle)** | 0.1-0.2% | **0%** | Zero |

## Features

### What Works
- ✅ White glowing frame around screen edge
- ✅ Click-through transparent overlay
- ✅ Adjustable brightness (20% - 100%)
- ✅ System tray icon with context menu
- ✅ Global hotkeys (Ctrl+Shift+L, Up, Down)
- ✅ Multi-monitor support
- ✅ Rounded corners (100px radius)
- ✅ Respects taskbar area
- ✅ Toggle light on/off

### What's Different
- ❌ No smooth anti-aliasing (GDI limitation)
- ❌ No glow effect layers (just one solid frame)
- ❌ No floating control panel (use hotkeys or tray menu instead)
- ❌ No hardware acceleration (CPU rendering is fast enough)

### Hotkeys
- **Ctrl + Shift + L** - Toggle light on/off
- **Ctrl + Shift + ↑** - Increase brightness
- **Ctrl + Shift + ↓** - Decrease brightness

## Quick Start

### Download & Run
1. Download `WindowsEdgeLightNative.exe` from the releases
2. Double-click to run
3. Right-click the tray icon for options
4. Use hotkeys to control

That's it. No installer, no admin rights needed, no dependencies to install.

## Building from Source

### Simplest Way (Command Line)
```cmd
cl /O1 /Os /MT /GL /GS- main.cpp WindowsEdgeLightNative.res ^
   /link /SUBSYSTEM:WINDOWS /LTCG user32.lib gdi32.lib shell32.lib dwmapi.lib
```

### Or Use MSBuild
```powershell
msbuild WindowsEdgeLightNative.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Or Use the Build Script
```powershell
.\build.ps1
```

### Prerequisites
- Windows 10 or later
- Visual Studio Build Tools 2022 (or full Visual Studio)
- Windows SDK 10.0+

That's it. No CMake, no vcpkg, no package managers, no nonsense.

## How It Works

### The Secret Sauce
1. Create a full-screen layered window with `WS_EX_LAYERED | WS_EX_TRANSPARENT`
2. Set black as the transparent color key with `SetLayeredWindowAttributes`
3. Use GDI regions to create the frame shape:
   - Outer rounded rectangle (screen bounds)
   - Inner rounded rectangle (inset by frame thickness)
   - Combine with `RGN_DIFF` to get just the frame
4. Fill the region with white and let black show through as transparent
5. Paint only when brightness changes (no continuous rendering)

**Result**: ~2KB of memory per frame (just the region data) + Windows GDI cache

### Why So Small?

**No Modern Graphics APIs**
- No Direct2D (saves 60-70MB runtime)
- No DirectX backbuffers (saves 32MB+ for 4K displays)
- No DirectWrite font rendering (saves 10-15MB)

**No Dynamic Allocations**
- Fixed 8-monitor array (not `std::vector`)
- No memory DC or bitmaps
- Direct window HDC rendering
- Regions created/destroyed on-demand only

**Aggressive Optimization**
- `/O1 /Os` - Optimize for size
- `/GL /LTCG` - Whole program optimization
- `/GS-` - No security cookies (no buffers to overflow)
- Function merging with `/OPT:ICF=5`
- Section merging (`.rdata` → `.text`)

## Architecture

```
main.cpp (385 lines)
  ├─ EdgeLightWindow class (all logic)
  ├─ WndProc (message handler)
  └─ wWinMain (entry point)

Dependencies:
  ├─ user32.lib    (Window management)
  ├─ gdi32.lib     (Graphics)
  ├─ shell32.lib   (Tray icon)
  └─ dwmapi.lib    (DWM integration)

No runtime DLLs needed (static linking)
```

## Technical Details

### Rendering Pipeline
```cpp
OnPaint() {
    // 1. Fill entire window with black (transparent color key)
    FillRect(hdc, &rc, BLACK_BRUSH);
    
    // 2. Create frame regions
    outerRegion = CreateRoundRectRgn(...);  // Screen bounds
    innerRegion = CreateRoundRectRgn(...);  // Inset by 80px
    frameRegion = outerRegion - innerRegion;  // CombineRgn
    
    // 3. Fill frame with white (adjusted by brightness)
    whiteBrush = CreateSolidBrush(RGB(intensity, intensity, intensity));
    FillRgn(hdc, frameRegion, whiteBrush);
    
    // 4. Cleanup
    DeleteObject(whiteBrush);
    DeleteObject(frameRegion);
    DeleteObject(innerRegion);
    DeleteObject(outerRegion);
}
```

Total memory allocated per paint: **~2KB** (just the region structures)

### Memory Breakdown
```
Private Memory: ~2 MB total
  ├─ Executable code: ~100 KB
  ├─ Static data: ~10 KB
  ├─ Stack: ~1 MB
  ├─ GDI objects: ~500 KB
  └─ Heap: ~400 KB (minimal)

Working Set: ~45 MB total
  ├─ Our private memory: 2 MB
  ├─ Shared GDI libraries: ~30 MB (shared across all apps)
  ├─ Kernel graphics drivers: ~10 MB (shared)
  └─ Window manager buffers: ~3 MB (shared)
```

**Working set is high but mostly shared - we only own 2MB!**

## Files

```
WindowsEdgeLightNative/
├── main.cpp                    # GDI version (current)
├── main_gdi.cpp                # Same as main.cpp (reference)
├── main_d2d.cpp.bak            # Direct2D version (backup, 135MB RAM)
├── resource.h                  # Resource IDs
├── WindowsEdgeLightNative.rc   # Icon + version info
├── WindowsEdgeLightNative.vcxproj  # Visual Studio project
├── ringlight.ico               # App icon
├── build.ps1                   # Build script
├── .gitignore                  # Excludes build outputs
├── README.md                   # This file
└── OPTIMIZATION_RESULTS.md     # Detailed comparison
```

## Compatibility

- ✅ Windows 10 (1809+)
- ✅ Windows 11
- ✅ x64 architecture
- ✅ All display resolutions (tested up to 4K)
- ✅ High DPI displays
- ✅ Multiple monitors (up to 8)

## FAQ

**Q: Why not use Direct2D for smoother rendering?**  
A: Direct2D allocates 32MB+ for backbuffers and loads 60-70MB of runtime DLLs. For drawing a simple rounded rectangle, GDI is 150x more memory efficient.

**Q: Why 80px frame thickness?**  
A: It's thick enough to be visible but thin enough not to obscure content. You can adjust `FRAME_THICKNESS` in the code.

**Q: Can I change the color?**  
A: Currently hardcoded to white. Modify the `RGB(intensity, intensity, intensity)` line in `OnPaint()` to change colors.

**Q: Why no GUI settings?**  
A: GUI = more code = more memory. Use hotkeys or edit the constants in the code and rebuild.

**Q: Does it work with HDR displays?**  
A: Yes, but color might look different. GDI doesn't do HDR color management.

**Q: Can I run multiple instances?**  
A: Technically yes, but why would you want overlapping frames?

## Comparison with Other Versions

### This Version (GDI)
- **Best for**: Minimalists, resource-constrained systems, learning Win32
- **Pros**: Tiny, fast, zero dependencies
- **Cons**: Less smooth rendering, no glow effect

### Previous Version (Direct2D - main_d2d.cpp.bak)
- **Best for**: Users who want smooth anti-aliasing
- **Pros**: Hardware accelerated, smooth curves
- **Cons**: 135MB memory, DirectX dependency

### Original WPF Version
- **Best for**: Users who want rich UI and settings
- **Pros**: Beautiful UI, many customization options
- **Cons**: 72MB + .NET runtime, 300MB RAM

## Contributing

Found a way to make it even smaller? Pull requests welcome! 

Guidelines:
- Keep it under 500 lines
- No heap allocations in hot paths
- No external dependencies (Win32 API only)
- Maintain <5MB private memory usage

## Credits

- Original concept: Scott Hanselman ([hanselminutes.com](https://hanselminutes.com))
- Ultra-minimal rewrite: GitHub Copilot + Dan Bosscher
- Inspired by the quest for the tiniest possible Windows overlay

## License

MIT License - Do whatever you want with it. No warranties, no guarantees.

---

**Made with ❤️ and an unhealthy obsession with tiny executables.**

*"640KB ought to be enough for anybody." - Probably not Bill Gates, but definitely this app.*
