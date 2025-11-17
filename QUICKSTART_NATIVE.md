# Quick Start - Native Edition

## Download & Run (End Users)

1. **Download** the executable:
   - Get `WindowsEdgeLightNative.exe` from releases
   - Or from `publish-native\` folder after building

2. **Run** it:
   - Double-click the exe
   - Edge light appears immediately
   - No installation needed!

3. **Use** it:
   - Press `Ctrl+Shift+L` to toggle
   - Press `Ctrl+Shift+↑` to brighten
   - Press `Ctrl+Shift+↓` to dim
   - Right-click tray icon for menu

## Build from Source (Developers)

### Prerequisites
```powershell
# Install Visual Studio Build Tools
winget install Microsoft.VisualStudio.2022.BuildTools --silent --override "--wait --quiet --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

### Build
```powershell
# Clone repo
git clone https://github.com/shanselman/WindowsEdgeLight.git
cd WindowsEdgeLight

# Build native version
.\build-native.ps1

# Output: publish-native\WindowsEdgeLightNative-v2.0-win-x64.exe
```

### Test
```powershell
# Automated tests
.\test-native.ps1

# Interactive test
.\test-native.ps1 -Interactive
```

## Key Features

### What It Does
- Creates glowing white border around your monitor
- Click-through (doesn't block mouse)
- Always on top
- Hardware accelerated (Direct2D)

### Controls
- **Ctrl+Shift+L** - Toggle on/off
- **Ctrl+Shift+↑** - Brighter
- **Ctrl+Shift+↓** - Dimmer
- **Tray Icon** - Right-click for menu
- **Control Panel** - Bottom center buttons

### Multi-Monitor
- Starts on primary monitor
- Right-click tray → "Switch Monitor"
- Or use control panel button

## Why Native?

| Feature | Benefit |
|---------|---------|
| **123 KB** | 600x smaller than WPF |
| **~40 MB RAM** | 7x less memory |
| **<0.5s startup** | 4x faster launch |
| **No dependencies** | Works anywhere |

## Troubleshooting

**Q: Nothing appears when I run it**  
A: Check system tray for icon. The overlay is transparent and might blend in.

**Q: Hotkeys don't work**  
A: Another app might be using those keys. Check tray menu instead.

**Q: Won't run on Windows 7**  
A: Requires Windows 10 or later for Direct2D 1.1

**Q: High memory usage?**  
A: Direct2D allocates texture memory for screen buffer. ~40MB is normal for full-screen overlay.

**Q: Can I customize colors?**  
A: Not yet - current version is white only. Feature request welcome!

## Comparison to WPF Version

Both versions have identical features, just different implementations:

| Aspect | WPF | Native |
|--------|-----|--------|
| Features | Same | Same |
| Visual | Same | Same |
| Hotkeys | Same | Same |
| Size | 72 MB | 123 KB |
| RAM | 300 MB | 40 MB |
| Updates | Auto | Manual |

Choose based on your priorities:
- **Want auto-updates?** → Use WPF
- **Want tiny & portable?** → Use Native

## More Info

- **Full Docs**: [NATIVE_VERSION.md](NATIVE_VERSION.md)
- **Comparison**: [FEATURE_COMPARISON.md](FEATURE_COMPARISON.md)
- **Technical**: [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)
- **WPF Version**: [README.md](README.md)

## Support

- **Issues**: [GitHub Issues](https://github.com/shanselman/WindowsEdgeLight/issues)
- **Discussions**: [GitHub Discussions](https://github.com/shanselman/WindowsEdgeLight/discussions)

---

**Created by**: Scott Hanselman  
**Version**: 2.0 Native  
**License**: Personal and educational use
