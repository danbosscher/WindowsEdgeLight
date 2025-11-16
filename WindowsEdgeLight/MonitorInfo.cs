using System.Runtime.InteropServices;

namespace WindowsEdgeLight;

/// <summary>
/// Provides monitor information using Win32 APIs to avoid dependency on Windows Forms
/// </summary>
public static class MonitorInfo
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;

        public int Width => Right - Left;
        public int Height => Bottom - Top;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MONITORINFO
    {
        public uint cbSize;
        public RECT rcMonitor;
        public RECT rcWork;
        public uint dwFlags;
    }

    private const uint MONITORINFOF_PRIMARY = 0x00000001;

    [DllImport("user32.dll")]
    private static extern bool EnumDisplayMonitors(IntPtr hdc, IntPtr lprcClip, MonitorEnumDelegate lpfnEnum, IntPtr dwData);

    [DllImport("user32.dll")]
    private static extern bool GetMonitorInfo(IntPtr hMonitor, ref MONITORINFO lpmi);

    private delegate bool MonitorEnumDelegate(IntPtr hMonitor, IntPtr hdcMonitor, ref RECT lprcMonitor, IntPtr dwData);

    public class Monitor
    {
        public RECT Bounds { get; set; }
        public RECT WorkingArea { get; set; }
        public bool Primary { get; set; }
    }

    private static List<Monitor> _monitors = new();

    public static Monitor[] AllMonitors
    {
        get
        {
            _monitors.Clear();
            EnumDisplayMonitors(IntPtr.Zero, IntPtr.Zero, MonitorEnum, IntPtr.Zero);
            return _monitors.ToArray();
        }
    }

    public static Monitor? PrimaryMonitor
    {
        get
        {
            var monitors = AllMonitors;
            return monitors.FirstOrDefault(m => m.Primary);
        }
    }

    private static bool MonitorEnum(IntPtr hMonitor, IntPtr hdcMonitor, ref RECT lprcMonitor, IntPtr dwData)
    {
        MONITORINFO mi = new MONITORINFO();
        mi.cbSize = (uint)Marshal.SizeOf(mi);

        if (GetMonitorInfo(hMonitor, ref mi))
        {
            _monitors.Add(new Monitor
            {
                Bounds = mi.rcMonitor,
                WorkingArea = mi.rcWork,
                Primary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0
            });
        }

        return true;
    }

    public static bool ContainsPoint(this RECT rect, int x, int y)
    {
        return x >= rect.Left && x < rect.Right && y >= rect.Top && y < rect.Bottom;
    }
}
