# Windows Edge Light

A lightweight native Windows application that displays a glowing frame around your screen edge. Built with Win32 and GDI for minimal resource usage.

![Windows Edge Light Demo](HeroImage.png)

## Performance vs Original

This native rewrite is dramatically more efficient than the original WPF version:

| Metric | Original WPF | Native (This Version) | Improvement |
|--------|-------------|----------------------|-------------|
| **Executable Size** | 72 MB | **109 KB** | **660x smaller** |
| **Memory Usage** | ~300 MB | **~4 MB** | **75x less** |
| **Startup Time** | ~800ms | **<80ms** | **10x faster** |
| **Runtime Dependencies** | .NET 10 + WPF | **None** | No runtime needed |
| **CPU Usage (idle)** | 0.1-0.2% | **0%** | Zero background CPU |

Same features, 660x smaller footprint.

## Features

- Click-through transparent overlay
- **Control panel with visual sliders** (toggleable)
- **Adjustable frame thickness** (20-150px via slider)
- **Blur/glow effect** around the frame
- Adjustable brightness levels
- System tray integration with context menu
- Global keyboard shortcuts
- Multi-monitor support
- Rounded corner frame design
- Minimal resource usage (~4MB RAM)

## Keyboard Shortcuts

- **Ctrl + Shift + L** - Toggle light on/off
- **Ctrl + Shift + C** - Toggle control panel
- **Ctrl + Shift + ↑** - Increase brightness
- **Ctrl + Shift + ↓** - Decrease brightness

## Installation

1. Download `WindowsEdgeLightNative.exe` from the [Releases](../../releases) page
2. Run the executable (no installation required)
3. The application will appear in the system tray
4. Right-click the tray icon to access settings

## Requirements

- Windows 10 or later
- No additional dependencies required

## Building from Source

### Prerequisites
- Windows 10 or later
- Visual Studio 2022 (or Build Tools)
- Windows SDK 10.0+

### Build Methods

**Using MSBuild:**
```powershell
msbuild WindowsEdgeLightNative.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Using the build script:**
```powershell
.\build.ps1
```

**Using Visual Studio:**
Open `WindowsEdgeLightNative.vcxproj` in Visual Studio and build the Release configuration.

## Technical Details

### Implementation
The application uses Win32 APIs and GDI for rendering:
- Layered windows for transparency (`WS_EX_LAYERED`)
- Click-through behavior (`WS_EX_TRANSPARENT`)
- GDI regions for rounded rectangle rendering
- Color key transparency for efficient compositing
- Direct window HDC rendering (no backbuffers)

### Performance Characteristics
- Executable size: ~109 KB
- Private memory usage: ~4 MB
- No runtime dependencies
- Startup time: <80ms
- Idle CPU usage: 0%

## Architecture

The application consists of a single C++ source file with the following components:
- `EdgeLightWindow` class - Main application logic
- Window procedure for message handling
- System tray integration
- Global hotkey registration

### Dependencies
- `user32.lib` - Window management
- `gdi32.lib` - Graphics rendering
- `shell32.lib` - System tray
- `dwmapi.lib` - Desktop Window Manager integration

Static linking ensures no runtime DLL dependencies.

## Project Structure

```
├── main.cpp                         # Main application source
├── resource.h                       # Resource definitions
├── WindowsEdgeLightNative.rc        # Resource script
├── WindowsEdgeLightNative.vcxproj   # Visual Studio project
├── ringlight.ico                    # Application icon
├── build.ps1                        # Build automation script
└── README.md                        # Documentation
```

## Compatibility

- Windows 10 (1809+) or Windows 11
- x64 architecture
- Multiple monitor support (up to 8 displays)
- High DPI aware

## Contributing

Contributions are welcome. Please ensure:
- Code follows existing style conventions
- Changes maintain minimal resource usage
- No additional external dependencies

## License

MIT License - See LICENSE file for details.

## Credits

Original concept by Scott Hanselman.
