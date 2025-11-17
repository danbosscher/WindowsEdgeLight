#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <commctrl.h>

#pragma comment(lib, "dwmapi")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "msimg32")
#pragma comment(lib, "comctl32")

#include "resource.h"

// Menu IDs
#define IDM_EXIT 103
#define IDM_TOGGLE 104
#define IDM_BRIGHTNESS_UP 105
#define IDM_BRIGHTNESS_DOWN 106
#define IDM_SWITCH_MONITOR 107
#define IDM_HELP 108
#define IDM_TOGGLE_CONTROLS 109

// Control IDs
#define IDC_THICKNESS_SLIDER 1001
#define IDC_BRIGHTNESS_SLIDER 1002
#define IDC_TOGGLE_BTN 1003
#define IDC_MONITOR_BTN 1004
#define IDC_CLOSE_BTN 1005

class EdgeLightWindow
{
private:
    HWND hwnd;
    HWND controlHwnd;
    NOTIFYICONDATA nid;
    bool isLightOn;
    int currentOpacity;
    int currentMonitorIndex;
    int frameThickness;
    HMONITOR monitors[8];
    int monitorCount;
    bool controlsVisible;
    
    static constexpr int OPACITY_STEP = 38;
    static constexpr int MIN_OPACITY = 51;
    static constexpr int MAX_OPACITY = 255;
    static constexpr int MIN_THICKNESS = 20;
    static constexpr int MAX_THICKNESS = 150;
    static constexpr int DEFAULT_THICKNESS = 80;
    static constexpr int CORNER_RADIUS = 100;
    static constexpr int BLUR_SIZE = 10;
    static constexpr int HOTKEY_TOGGLE = 1;
    static constexpr int HOTKEY_BRIGHTNESS_UP = 2;
    static constexpr int HOTKEY_BRIGHTNESS_DOWN = 3;
    static constexpr int HOTKEY_TOGGLE_CONTROLS = 4;

public:
    EdgeLightWindow() : 
        hwnd(nullptr),
        controlHwnd(nullptr),
        isLightOn(true),
        currentOpacity(255),
        currentMonitorIndex(0),
        monitorCount(0),
        frameThickness(DEFAULT_THICKNESS),
        controlsVisible(true)
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
        InitCommonControls();
        EnumerateMonitors();
        if (CreateOverlayWindow() != S_OK)
            return E_FAIL;
        
        CreateControlWindow();
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

    HRESULT CreateControlWindow()
    {
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = EdgeLightWindow::ControlWndProc;
        wcex.hInstance = GetModuleHandle(nullptr);
        wcex.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = L"EdgeLightControlClass";

        RegisterClassEx(&wcex);

        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(monitors[currentMonitorIndex], &mi);
        RECT workArea = mi.rcWork;

        int controlWidth = 320;
        int controlHeight = 140;
        int controlX = workArea.left + (workArea.right - workArea.left - controlWidth) / 2;
        int controlY = workArea.bottom - controlHeight - 80;

        controlHwnd = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
            L"EdgeLightControlClass",
            L"Edge Light Controls",
            WS_POPUP | WS_VISIBLE,
            controlX, controlY,
            controlWidth, controlHeight,
            hwnd,
            nullptr,
            GetModuleHandle(nullptr),
            this
        );

        if (!controlHwnd)
            return E_FAIL;

        SetLayeredWindowAttributes(controlHwnd, 0, 230, LWA_ALPHA);
        
        // Create controls
        CreateControlWidgets();
        
        ShowWindow(controlHwnd, controlsVisible ? SW_SHOW : SW_HIDE);
        return S_OK;
    }

    void CreateControlWidgets()
    {
        HINSTANCE hInst = GetModuleHandle(nullptr);
        HWND hwndParent = controlHwnd;
        
        // Thickness slider
        CreateWindow(L"STATIC", L"Frame Thickness:", WS_CHILD | WS_VISIBLE,
            10, 10, 120, 20, hwndParent, nullptr, hInst, nullptr);
        
        HWND thicknessSlider = CreateWindow(TRACKBAR_CLASS, L"",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            130, 10, 150, 25, hwndParent, (HMENU)IDC_THICKNESS_SLIDER, hInst, nullptr);
        
        SendMessage(thicknessSlider, TBM_SETRANGE, TRUE, MAKELONG(MIN_THICKNESS, MAX_THICKNESS));
        SendMessage(thicknessSlider, TBM_SETPOS, TRUE, frameThickness);
        SendMessage(thicknessSlider, TBM_SETTICFREQ, 10, 0);
        
        // Brightness slider
        CreateWindow(L"STATIC", L"Brightness:", WS_CHILD | WS_VISIBLE,
            10, 45, 120, 20, hwndParent, nullptr, hInst, nullptr);
        
        HWND brightnessSlider = CreateWindow(TRACKBAR_CLASS, L"",
            WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
            130, 45, 150, 25, hwndParent, (HMENU)IDC_BRIGHTNESS_SLIDER, hInst, nullptr);
        
        SendMessage(brightnessSlider, TBM_SETRANGE, TRUE, MAKELONG(MIN_OPACITY, MAX_OPACITY));
        SendMessage(brightnessSlider, TBM_SETPOS, TRUE, currentOpacity);
        
        // Buttons
        CreateWindow(L"BUTTON", L"Toggle", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 85, 70, 30, hwndParent, (HMENU)IDC_TOGGLE_BTN, hInst, nullptr);
        
        if (monitorCount > 1)
        {
            CreateWindow(L"BUTTON", L"Monitor", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                90, 85, 70, 30, hwndParent, (HMENU)IDC_MONITOR_BTN, hInst, nullptr);
        }
        
        CreateWindow(L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            monitorCount > 1 ? 170 : 90, 85, 70, 30, hwndParent, (HMENU)IDC_CLOSE_BTN, hInst, nullptr);
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
        RegisterHotKey(hwnd, HOTKEY_TOGGLE_CONTROLS, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, 'C');
    }

    void OnPaint()
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        // Fill with black (transparent color key)
        HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
        FillRect(hdc, &rc, blackBrush);
        
        if (isLightOn)
        {
            int innerRadius = CORNER_RADIUS - frameThickness;
            if (innerRadius < 10) innerRadius = 10;
            
            // Create main frame region
            HRGN outerRegion = CreateRoundRectRgn(20, 20, rc.right - 20, rc.bottom - 20, 
                                                   CORNER_RADIUS * 2, CORNER_RADIUS * 2);
            HRGN innerRegion = CreateRoundRectRgn(20 + frameThickness, 20 + frameThickness,
                                                   rc.right - 20 - frameThickness, 
                                                   rc.bottom - 20 - frameThickness,
                                                   innerRadius * 2, innerRadius * 2);
            
            HRGN frameRegion = CreateRectRgn(0, 0, 0, 0);
            CombineRgn(frameRegion, outerRegion, innerRegion, RGN_DIFF);
            
            // Draw blur/glow layers first (from outside to inside for proper alpha blending)
            int intensity = (255 * currentOpacity) / 255;
            
            for (int i = BLUR_SIZE; i >= 1; i -= 2)
            {
                int offset = i;
                // Blur should fade out as it goes outward
                int blurIntensity = (intensity * (BLUR_SIZE - i + 1)) / (BLUR_SIZE + 4);
                
                // Create blur ring just outside the main frame
                HRGN blurOuter = CreateRoundRectRgn(
                    20 - offset, 20 - offset,
                    rc.right - 20 + offset, rc.bottom - 20 + offset,
                    (CORNER_RADIUS + offset) * 2, (CORNER_RADIUS + offset) * 2);
                    
                HRGN blurInner = CreateRoundRectRgn(
                    20 - offset + 2, 20 - offset + 2,
                    rc.right - 20 + offset - 2, rc.bottom - 20 + offset - 2,
                    (CORNER_RADIUS + offset - 2) * 2, (CORNER_RADIUS + offset - 2) * 2);
                
                HRGN blurRegion = CreateRectRgn(0, 0, 0, 0);
                CombineRgn(blurRegion, blurOuter, blurInner, RGN_DIFF);
                
                HBRUSH blurBrush = CreateSolidBrush(RGB(blurIntensity, blurIntensity, blurIntensity));
                FillRgn(hdc, blurRegion, blurBrush);
                
                DeleteObject(blurBrush);
                DeleteObject(blurRegion);
                DeleteObject(blurInner);
                DeleteObject(blurOuter);
            }
            
            // Draw main frame on top
            HBRUSH mainBrush = CreateSolidBrush(RGB(intensity, intensity, intensity));
            FillRgn(hdc, frameRegion, mainBrush);
            
            // Cleanup
            DeleteObject(mainBrush);
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
            UpdateBrightnessSlider();
        }
    }

    void DecreaseBrightness()
    {
        if (currentOpacity > MIN_OPACITY)
        {
            currentOpacity = max(MIN_OPACITY, currentOpacity - OPACITY_STEP);
            InvalidateRect(hwnd, nullptr, FALSE);
            UpdateBrightnessSlider();
        }
    }

    void SetBrightness(int value)
    {
        currentOpacity = max(MIN_OPACITY, min(MAX_OPACITY, value));
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void SetFrameThickness(int value)
    {
        frameThickness = max(MIN_THICKNESS, min(MAX_THICKNESS, value));
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void UpdateBrightnessSlider()
    {
        if (controlHwnd)
        {
            HWND slider = GetDlgItem(controlHwnd, IDC_BRIGHTNESS_SLIDER);
            if (slider)
            {
                SendMessage(slider, TBM_SETPOS, TRUE, currentOpacity);
            }
        }
    }

    void ToggleControls()
    {
        controlsVisible = !controlsVisible;
        if (controlHwnd)
        {
            ShowWindow(controlHwnd, controlsVisible ? SW_SHOW : SW_HIDE);
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
        RepositionControlWindow();
    }

    void RepositionControlWindow()
    {
        if (!controlHwnd) return;

        RECT mainRect;
        GetWindowRect(hwnd, &mainRect);
        
        RECT ctrlRect;
        GetWindowRect(controlHwnd, &ctrlRect);
        int controlWidth = ctrlRect.right - ctrlRect.left;
        int controlHeight = ctrlRect.bottom - ctrlRect.top;
        
        int controlX = mainRect.left + (mainRect.right - mainRect.left - controlWidth) / 2;
        int controlY = mainRect.bottom - controlHeight - 80;

        SetWindowPos(controlHwnd, HWND_TOPMOST, controlX, controlY, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    }

    void ShowTrayMenu()
    {
        POINT pt;
        GetCursorPos(&pt);

        HMENU hMenu = CreatePopupMenu();
        AppendMenu(hMenu, MF_STRING, IDM_HELP, L"Keyboard Shortcuts");
        AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenu(hMenu, MF_STRING, IDM_TOGGLE, L"Toggle Light (Ctrl+Shift+L)");
        AppendMenu(hMenu, MF_STRING, IDM_TOGGLE_CONTROLS, L"Toggle Controls (Ctrl+Shift+C)");
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
            L"Toggle Controls:  Ctrl + Shift + C\n"
            L"Brightness Up:  Ctrl + Shift + \x2191\n"
            L"Brightness Down:  Ctrl + Shift + \x2193\n\n"
            L"Features:\n"
            L"\x2022 Click-through overlay\n"
            L"\x2022 Adjustable frame thickness\n"
            L"\x2022 Blur/glow effect\n"
            L"\x2022 Multi-monitor support\n\n"
            L"Original concept by Scott Hanselman\n"
            L"Version 2.1 - Enhanced Edition",
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
                case HOTKEY_TOGGLE_CONTROLS:
                    pThis->ToggleControls();
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
                case IDM_TOGGLE_CONTROLS:
                    pThis->ToggleControls();
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
                UnregisterHotKey(hwnd, HOTKEY_TOGGLE_CONTROLS);
                PostQuitMessage(0);
                return 0;
            }
        }

        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    static LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
            case WM_HSCROLL:
            {
                if (lParam)
                {
                    int id = GetDlgCtrlID((HWND)lParam);
                    int pos = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
                    
                    if (id == IDC_THICKNESS_SLIDER)
                    {
                        pThis->SetFrameThickness(pos);
                    }
                    else if (id == IDC_BRIGHTNESS_SLIDER)
                    {
                        pThis->SetBrightness(pos);
                    }
                }
                return 0;
            }

            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                case IDC_TOGGLE_BTN:
                    pThis->ToggleLight();
                    return 0;
                case IDC_MONITOR_BTN:
                    pThis->SwitchMonitor();
                    return 0;
                case IDC_CLOSE_BTN:
                    pThis->ToggleControls();
                    return 0;
                }
                break;
            }
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
