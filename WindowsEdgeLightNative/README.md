# Windows Edge Light - Native Edition

A lightweight, native Win32/Direct2D implementation of Windows Edge Light.

## Performance Comparison

| Version | Executable Size | Private Memory | Technology |
|---------|----------------|--------------|------------|
| WPF Original | ~72 MB | ~300 MB RAM | .NET 10 + WPF |
| **Native (GDI)** | **~106 KB** | **~2 MB RAM** | **Win32 + GDI** |
| Native (D2D) | ~118 KB | ~135 MB RAM | Win32 + Direct2D |

## Features

- ✅ Click-through transparent overlay
- ✅ Pure GDI rendering (ultra-minimal memory)
- ✅ Adjustable brightness (20-100%)
- ✅ Global hotkeys (Ctrl+Shift+L, Up, Down)
- ✅ System tray integration
- ✅ Multi-monitor support with switching
- ✅ DPI-aware
- ✅ Respects taskbar area
- ✅ Rounded corners frame

## Building

### Option 1: Visual Studio (Recommended)

Open `WindowsEdgeLightNative.sln` in Visual Studio 2022 and build.

### Option 2: MSBuild

```powershell
msbuild WindowsEdgeLightNative.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Option 3: CMake

```powershell
.\build.ps1
```

Or manually:
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Option 4: Developer Command Prompt

```cmd
cl /nologo /EHsc /O2 /MT /std:c++20 /Fe:WindowsEdgeLightNative.exe main.cpp ^
   /link /SUBSYSTEM:WINDOWS d2d1.lib dwrite.lib windowscodecs.lib ^
   comctl32.lib user32.lib gdi32.lib shell32.lib ole32.lib
```

## Prerequisites

- Windows 10 or later
- Visual Studio 2022 with C++ Desktop Development, OR
- Visual Studio Build Tools 2022, OR  
- Windows SDK 10.0+ with any C++ compiler

## Architecture

- **Win32 API**: Window management, input handling
- **GDI**: Lightweight 2D graphics (regions, brushes)
- **Static linking**: No runtime dependencies
- **Zero COM**: No component initialization overhead
- **Pure C++17**: No STL containers (fixed arrays only)

## Technical Details

The application uses:
- Layered windows with `WS_EX_LAYERED` for transparency
- `WS_EX_TRANSPARENT` for click-through behavior
- GDI regions (CreateRoundRectRgn + CombineRgn) for rounded frame
- Direct rendering to window HDC (no memory DC overhead)
- Global hotkey registration via `RegisterHotKey`
- System tray icon with context menu
- Monitor enumeration for multi-display support
- Color key transparency (LWA_COLORKEY) for zero-copy rendering

## Code Structure

- `main.cpp`: Complete application (single file)
- `resource.h`: Resource definitions
- `WindowsEdgeLightNative.rc`: Resource script (icon, version info)
- `ringlight.ico`: Application icon

## Optimizations

- Static CRT linking (`/MT`) eliminates runtime DLL dependencies
- Pure GDI with no Direct2D/DirectX overhead
- Fixed arrays instead of std::vector (zero heap allocations)
- No memory DC or backbuffer (direct rendering)
- Aggressive compiler flags (`/O1 /Os /GL /LTCG`)
- Function merging and dead code elimination
- Section merging (`.rdata` into `.text`)
- **Result**: 106KB executable, ~2MB private memory

## License

Same as Windows Edge Light - provided as-is for personal and educational use.
