#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <shellapi.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "msimg32")

#include "resource.h"

class EdgeLightWindow
{
private:
    HWND hwnd;
    NOTIFYICONDATA nid;
    bool isLightOn;
    int currentOpacity;
    int currentMonitorIndex;
    HMONITOR monitors[8];
    int monitorCount;
    
    static constexpr int OPACITY_STEP = 38;
    static constexpr int MIN_OPACITY = 51;
    static constexpr int MAX_OPACITY = 255;
    static constexpr int FRAME_THICKNESS = 80;
    static constexpr int CORNER_RADIUS = 100;
    static constexpr int HOTKEY_TOGGLE = 1;
    static constexpr int HOTKEY_BRIGHTNESS_UP = 2;
    static constexpr int HOTKEY_BRIGHTNESS_DOWN = 3;

public:
    EdgeLightWindow() : 
        hwnd(nullptr),
        isLightOn(true),
        currentOpacity(255),
        currentMonitorIndex(0),
        monitorCount(0)
    {
        ZeroMemory(&nid, sizeof(nid));
        ZeroMemory(monitors, sizeof(monitors));
    }

    ~EdgeLightWindow()
    {
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }

    HRESULT Initialize()
    {
        EnumerateMonitors();
        if (CreateOverlayWindow() != S_OK)
            return E_FAIL;
        
        SetupTrayIcon();
        RegisterHotKeys();
        return S_OK;
    }

    void RunMessageLoop()
    {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

private:
    static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData)
    {
        auto* pThis = reinterpret_cast<EdgeLightWindow*>(dwData);
        if (pThis->monitorCount < 8)
        {
            pThis->monitors[pThis->monitorCount++] = hMonitor;
        }
        return TRUE;
    }

    void EnumerateMonitors()
    {
        monitorCount = 0;
        EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(this));
        
        POINT pt = {0, 0};
        HMONITOR primaryMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
        for (int i = 0; i < monitorCount; i++)
        {
            if (monitors[i] == primaryMonitor)
            {
                currentMonitorIndex = i;
                break;
            }
        }
    }

    HRESULT CreateOverlayWindow()
    {
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = EdgeLightWindow::WndProc;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = L"EdgeLightWindowClass";
        wcex.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_APPICON));
        wcex.hIconSm = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_APPICON));

        RegisterClassEx(&wcex);

        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(monitors[currentMonitorIndex], &mi);
        RECT workArea = mi.rcWork;

        hwnd = CreateWindowEx(
            WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            L"EdgeLightWindowClass",
            L"Windows Edge Light",
            WS_POPUP,
            workArea.left, workArea.top,
            workArea.right - workArea.left,
            workArea.bottom - workArea.top,
            nullptr,
            nullptr,
            GetModuleHandle(nullptr),
            this
        );

        if (!hwnd)
            return E_FAIL;

        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);

        return S_OK;
    }

    void SetupTrayIcon()
    {
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        nid.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TRAYICON));
        wcscpy_s(nid.szTip, L"Windows Edge Light - Right-click for options");
        
        Shell_NotifyIcon(NIM_ADD, &nid);
    }

    void RegisterHotKeys()
    {
        RegisterHotKey(hwnd, HOTKEY_TOGGLE, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'L');
        RegisterHotKey(hwnd, HOTKEY_BRIGHTNESS_UP, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_UP);
        RegisterHotKey(hwnd, HOTKEY_BRIGHTNESS_DOWN, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_DOWN);
    }

    void OnPaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        // Always fill background with black (our transparent color key)
        HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
        FillRect(hdc, &rc, blackBrush);
        
        if (isLightOn)
        {
            // Create region for frame (outer - inner)
            HRGN outerRegion = CreateRoundRectRgn(20, 20, rc.right - 20, rc.bottom - 20, 
                                                   CORNER_RADIUS * 2, CORNER_RADIUS * 2);
            HRGN innerRegion = CreateRoundRectRgn(20 + FRAME_THICKNESS, 20 + FRAME_THICKNESS,
                                                   rc.right - 20 - FRAME_THICKNESS, 
                                                   rc.bottom - 20 - FRAME_THICKNESS,
                                                   (CORNER_RADIUS - FRAME_THICKNESS) * 2, 
                                                   (CORNER_RADIUS - FRAME_THICKNESS) * 2);
            
            HRGN frameRegion = CreateRectRgn(0, 0, 0, 0);
            CombineRgn(frameRegion, outerRegion, innerRegion, RGN_DIFF);
            
            // Calculate color with opacity (simple linear blend)
            int intensity = (255 * currentOpacity) / 255;
            HBRUSH whiteBrush = CreateSolidBrush(RGB(intensity, intensity, intensity));
            FillRgn(hdc, frameRegion, whiteBrush);
            
            // Cleanup
            DeleteObject(whiteBrush);
            DeleteObject(frameRegion);
            DeleteObject(innerRegion);
            DeleteObject(outerRegion);
        }
        
        EndPaint(hwnd, &ps);
    }

    void ToggleLight()
    {
        isLightOn = !isLightOn;
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void IncreaseBrightness()
    {
        if (currentOpacity < MAX_OPACITY)
        {
            currentOpacity = min(MAX_OPACITY, currentOpacity + OPACITY_STEP);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }

    void DecreaseBrightness()
    {
        if (currentOpacity > MIN_OPACITY)
        {
            currentOpacity = max(MIN_OPACITY, currentOpacity - OPACITY_STEP);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }

    void SwitchMonitor()
    {
        if (monitorCount <= 1) return;

        currentMonitorIndex = (currentMonitorIndex + 1) % monitorCount;
        
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(monitors[currentMonitorIndex], &mi);
        RECT workArea = mi.rcWork;

        SetWindowPos(hwnd, HWND_TOPMOST,
            workArea.left, workArea.top,
            workArea.right - workArea.left,
            workArea.bottom - workArea.top,
            SWP_SHOWWINDOW);

        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void ShowTrayMenu()
    {
        POINT pt;
        GetCursorPos(&pt);

        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, IDM_HELP, L"Keyboard Shortcuts");
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hMenu, MF_STRING, IDM_TOGGLE, L"Toggle Light (Ctrl+Shift+L)");
        AppendMenu(hMenu, MF_STRING, IDM_BRIGHTNESS_UP, L"Brightness Up (Ctrl+Shift+\x2191)");
        AppendMenu(hMenu, MF_STRING, IDM_BRIGHTNESS_DOWN, L"Brightness Down (Ctrl+Shift+\x2193)");
        
        if (monitorCount > 1)
        {
            AppendMenu(hMenu, MF_STRING, IDM_SWITCH_MONITOR, L"Switch Monitor");
        }
        
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");

        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
        DestroyMenu(hMenu);
    }

    void ShowHelp()
    {
        MessageBox(hwnd,
            L"Windows Edge Light - Keyboard Shortcuts\n\n"
            L"Toggle Light:  Ctrl + Shift + L\n"
            L"Brightness Up:  Ctrl + Shift + \x2191\n"
            L"Brightness Down:  Ctrl + Shift + \x2193\n\n"
            L"Features:\n"
            L"\x2022 Click-through overlay - won't interfere with your work\n"
            L"\x2022 Global hotkeys work from any application\n"
            L"\x2022 Right-click taskbar icon for menu\n\n"
            L"Created by Scott Hanselman\n"
            L"Version 2.0 - Ultra-Minimal GDI Edition",
            L"Windows Edge Light - Help",
            MB_OK | MB_ICONINFORMATION);
    }

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        EdgeLightWindow* pThis = nullptr;

        if (message == WM_NCCREATE)
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<EdgeLightWindow*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
        else
        {
            pThis = reinterpret_cast<EdgeLightWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (pThis)
        {
            switch (message)
            {
            case WM_PAINT:
                pThis->OnPaint();
                return 0;

            case WM_ERASEBKGND:
                return 1;

            case WM_HOTKEY:
                switch (wParam)
                {
                case HOTKEY_TOGGLE:
                    pThis->ToggleLight();
                    break;
                case HOTKEY_BRIGHTNESS_UP:
                    pThis->IncreaseBrightness();
                    break;
                case HOTKEY_BRIGHTNESS_DOWN:
                    pThis->DecreaseBrightness();
                    break;
                }
                return 0;

            case WM_USER + 1:
                if (lParam == WM_RBUTTONUP)
                {
                    pThis->ShowTrayMenu();
                }
                else if (lParam == WM_LBUTTONDBLCLK)
                {
                    pThis->ShowHelp();
                }
                return 0;

            case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                case IDM_EXIT:
                    PostQuitMessage(0);
                    return 0;
                case IDM_TOGGLE:
                    pThis->ToggleLight();
                    return 0;
                case IDM_BRIGHTNESS_UP:
                    pThis->IncreaseBrightness();
                    return 0;
                case IDM_BRIGHTNESS_DOWN:
                    pThis->DecreaseBrightness();
                    return 0;
                case IDM_SWITCH_MONITOR:
                    pThis->SwitchMonitor();
                    return 0;
                case IDM_HELP:
                    pThis->ShowHelp();
                    return 0;
                }
                break;

            case WM_DESTROY:
                UnregisterHotKey(hwnd, HOTKEY_TOGGLE);
                UnregisterHotKey(hwnd, HOTKEY_BRIGHTNESS_UP);
                UnregisterHotKey(hwnd, HOTKEY_BRIGHTNESS_DOWN);
                PostQuitMessage(0);
                return 0;
            }
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }
};

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    EdgeLightWindow app;
    if (SUCCEEDED(app.Initialize()))
    {
        app.RunMessageLoop();
    }
    return 0;
}
