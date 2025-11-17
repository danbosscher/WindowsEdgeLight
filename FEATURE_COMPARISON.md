# Feature Comparison: WPF vs Native

## Complete Feature Checklist

| Feature | WPF | Native | Notes |
|---------|-----|--------|-------|
| **Core Features** ||||
| Click-through overlay | ✅ | ✅ | Identical behavior |
| Hardware-accelerated rendering | ✅ | ✅ | Both use GPU |
| Adjustable brightness (20-100%) | ✅ | ✅ | Same range and steps |
| Toggle light on/off | ✅ | ✅ | |
| Rounded corners with glow | ✅ | ✅ | Same visual appearance |
| **Input & Control** ||||
| Ctrl+Shift+L (toggle) | ✅ | ✅ | |
| Ctrl+Shift+↑ (brightness up) | ✅ | ✅ | |
| Ctrl+Shift+↓ (brightness down) | ✅ | ✅ | |
| System tray icon | ✅ | ✅ | |
| System tray menu | ✅ | ✅ | Same menu items |
| Floating control panel | ✅ | ✅ | |
| Control panel hover effect | ✅ | ✅ | |
| Help dialog | ✅ | ✅ | |
| **Multi-Monitor** ||||
| Primary monitor detection | ✅ | ✅ | |
| Monitor switching | ✅ | ✅ | |
| Respects taskbar area | ✅ | ✅ | Uses WorkingArea |
| DPI-aware scaling | ✅ | ✅ | |
| **Advanced** ||||
| Auto-update system | ✅ | ❌ | Updatum library |
| Version info in file | ✅ | ✅ | |
| Custom application icon | ✅ | ✅ | |
| **Performance** ||||
| Executable size | 72 MB | **123 KB** | 600x smaller |
| Memory usage | ~300 MB | **~10 MB** | 30x less |
| Startup time | ~2 sec | **<0.1 sec** | 20x faster |
| Runtime dependencies | .NET 10 | **None** | Built-in APIs |

## Visual Comparison

Both versions produce **identical visual output**:
- Same rounded corner radius
- Same frame thickness (80px)
- Same gradient effect
- Same glow/blur appearance
- Same opacity range

## Code Comparison

### Lines of Code
- **WPF**: ~800 lines (split across multiple files + XAML)
- **Native**: ~700 lines (single C++ file)

### Complexity
- **WPF**: Higher-level abstractions, easier UI design
- **Native**: Lower-level, more manual control

## When to Use Which

### Use WPF Edition if you want:
- Automatic updates from GitHub Releases
- Don't mind larger download size
- Already have .NET installed
- Prefer managed code

### Use Native Edition if you want:
- Minimal system impact
- No dependencies
- Fastest possible startup
- Run from USB/portable
- Guaranteed long-term compatibility

## Migration

Switching between versions is seamless:
1. Close current version
2. Run new version
3. No settings or configuration to transfer
4. Both use same hotkeys and behavior

