# Testing Checklist for WindowsEdgeLight Optimizations

This document provides a comprehensive testing checklist to verify that the optimizations don't break any functionality.

## Pre-Testing: Build the Optimized Version

```powershell
# Using the build script
.\build.ps1

# Or manually
cd WindowsEdgeLight
dotnet publish -c Release -r win-x64 /p:DebugType=None /p:DebugSymbols=false --self-contained
```

The executable will be in: `WindowsEdgeLight\bin\Release\net10.0-windows\win-x64\publish\WindowsEdgeLight.exe`

## Measurements

### Before Optimization
- [ ] Executable size: ~72MB
- [ ] Memory usage (Task Manager): ~255.7MB

### After Optimization
- [ ] Executable size: _______ MB (Target: 40-50MB)
- [ ] Memory usage (Task Manager): _______ MB (Target: 150-180MB)
- [ ] Startup time: _______ seconds

## Functional Testing

### Core Functionality
- [ ] Application launches without errors
- [ ] Edge light appears around the primary monitor
- [ ] Edge light has the correct white gradient appearance
- [ ] Edge light has the soft blur/glow effect
- [ ] Window is click-through (clicks pass through to applications beneath)
- [ ] Window stays on top of other windows

### Monitor Detection
- [ ] Edge light displays on the primary monitor (single monitor setup)
- [ ] Edge light displays on the primary monitor (multi-monitor setup)
- [ ] Edge light respects taskbar area (doesn't overlap taskbar)
- [ ] Edge light has correct dimensions on high-DPI displays (150%, 200% scaling)

### Brightness Controls
- [ ] Brightness up button increases opacity
- [ ] Brightness down button decreases opacity
- [ ] Maximum brightness (100%) works correctly
- [ ] Minimum brightness (20%) works correctly
- [ ] Opacity changes are smooth and visible

### Toggle Functionality
- [ ] Toggle button turns light on/off
- [ ] Light state persists correctly between toggles
- [ ] Control window remains visible when light is off

### Multi-Monitor Support (if applicable)
- [ ] "Switch Monitor" button is enabled with multiple monitors
- [ ] "Switch Monitor" button is disabled with single monitor
- [ ] Switching monitors moves edge light to the next monitor
- [ ] Edge light dimensions adjust for different monitor resolutions
- [ ] Control window follows edge light when switching monitors

### Hotkeys
- [ ] `Ctrl+Shift+L` toggles the light on/off
- [ ] `Ctrl+Shift+Up` increases brightness
- [ ] `Ctrl+Shift+Down` decreases brightness
- [ ] Hotkeys work when other applications are focused
- [ ] Hotkeys don't conflict with other applications

### Taskbar Icon
- [ ] Taskbar icon appears with correct ring light icon
- [ ] Taskbar icon tooltip shows "Windows Edge Light - Right-click for options"
- [ ] Right-click shows context menu
- [ ] Context menu "📋 Keyboard Shortcuts" opens help dialog
- [ ] Context menu "💡 Toggle Light" works
- [ ] Context menu "🔆 Brightness Up" works
- [ ] Context menu "🔅 Brightness Down" works
- [ ] Context menu "✖ Exit" closes the application
- [ ] Double-click taskbar icon shows help dialog

### Help Dialog
- [ ] Help dialog displays version number correctly
- [ ] Help dialog shows all keyboard shortcuts
- [ ] Help dialog can be closed

### Control Window
- [ ] Control window appears at the bottom center of the edge light
- [ ] Control window buttons are visible and functional
- [ ] Control window repositions when monitor is switched
- [ ] Control window buttons have hover effects

### Update System (Updatum)
- [ ] Application checks for updates on startup (after 2 second delay)
- [ ] Update dialog appears if new version is available
- [ ] Download progress dialog shows during update download
- [ ] Update installation works correctly
- [ ] "Remind Later" and "Skip" buttons work

### Window Behavior
- [ ] Window handles screen resolution changes
- [ ] Window handles DPI changes
- [ ] Window handles monitor hot-plug/unplug
- [ ] Window repositions correctly when moved
- [ ] Window geometry updates correctly on size changes
- [ ] Application closes cleanly without errors

## Performance Testing

### Memory Usage
- [ ] Initial memory usage after startup: _______ MB
- [ ] Memory usage after 5 minutes: _______ MB
- [ ] Memory usage after 30 minutes: _______ MB
- [ ] Memory usage after toggling light 10 times: _______ MB
- [ ] Memory usage after switching monitors 10 times (if applicable): _______ MB
- [ ] No memory leaks detected (memory should stabilize)

### CPU Usage
- [ ] CPU usage at idle: _______ %
- [ ] CPU usage during animations (if any): _______ %
- [ ] CPU usage doesn't stay elevated at idle

### Startup Performance
- [ ] Application starts in under 3 seconds
- [ ] No noticeable delay in UI responsiveness

## Compatibility Testing

### Windows Versions
- [ ] Windows 10 (version 21H2 or later)
- [ ] Windows 11 (version 21H2 or later)

### DPI Scaling
- [ ] 100% scaling (96 DPI)
- [ ] 125% scaling (120 DPI)
- [ ] 150% scaling (144 DPI)
- [ ] 200% scaling (192 DPI)

### Monitor Configurations
- [ ] Single monitor (1920x1080)
- [ ] Single monitor (2560x1440)
- [ ] Single monitor (3840x2160, 4K)
- [ ] Dual monitors (horizontal arrangement)
- [ ] Dual monitors (vertical arrangement)
- [ ] Triple monitors (if available)

### Taskbar Positions
- [ ] Taskbar at bottom
- [ ] Taskbar at top
- [ ] Taskbar at left
- [ ] Taskbar at right
- [ ] Auto-hide taskbar

## Regression Testing

### Verify No Breaking Changes
- [ ] InvariantGlobalization doesn't affect number formatting (should use English-like format)
- [ ] Disabled EventSource doesn't cause crashes
- [ ] Disabled HttpActivity doesn't affect Updatum downloads
- [ ] Disabled MetadataUpdater doesn't affect app functionality
- [ ] Trimming doesn't remove necessary WPF types
- [ ] ReadyToRun doesn't cause runtime errors
- [ ] Custom MonitorInfo class works identically to Windows Forms Screen API

## Edge Cases

- [ ] Application handles no monitors gracefully (if possible to test)
- [ ] Application handles monitor disconnect while running
- [ ] Application handles very large monitors (5K, 8K)
- [ ] Application handles very small monitors (1024x768)
- [ ] Application handles mixed DPI setups
- [ ] Application handles rapid hotkey presses
- [ ] Application handles taskbar size changes

## Error Conditions

- [ ] Application doesn't crash on Win32 API failures
- [ ] Application handles icon file missing gracefully
- [ ] Application handles update check failures gracefully
- [ ] Application handles download failures gracefully

## Sign-Off

- [ ] All critical tests passed
- [ ] All high-priority tests passed
- [ ] Known issues documented below
- [ ] Performance improvements verified

### Known Issues
_Document any issues found during testing here_

### Performance Results
- **Executable Size**: _______ MB (______% reduction)
- **Memory Usage**: _______ MB (______% reduction)
- **Startup Time**: _______ seconds

### Tester Information
- Tester: _________________
- Date: _________________
- OS Version: _________________
- .NET Version: _________________
- Test Environment: _________________
