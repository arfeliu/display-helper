#include <windows.h>
#include <shellapi.h>
#include <vector>
#include "resource.h"

#pragma comment(lib,"Dxva2.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_EXIT 1001

std::vector<HANDLE> monitors;

int levels[] = {0, 20, 40, 60, 80, 100};
int levelIndex = 0;

// Set brightness via DDC/CI
void SetBrightnessAll(int value)
{
    for (auto &hMonitor : monitors)
    {
        SetMonitorBrightness(hMonitor, value);
    }
}

// Cycle predefined brightness levels
void CycleBrightness()
{
    levelIndex++;
    if (levelIndex >= 6) levelIndex = 0;
    SetBrightnessAll(levels[levelIndex]);
}

// Enumerate monitors and open handles for DDC/CI
BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM)
{
    DWORD count = 0;
    if (!GetNumberOfPhysicalMonitorsFromHMONITOR(hMon, &count))
        return TRUE;

    std::vector<PHYSICAL_MONITOR> temp(count);
    if (GetPhysicalMonitorsFromHMONITOR(hMon, count, temp.data()))
    {
        for (auto &m : temp)
        {
            monitors.push_back(m.hPhysicalMonitor);
        }
    }

    return TRUE;
}

// Initialize all monitors
void InitMonitors()
{
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);
}

// Cleanup all DDC handles
void CleanupMonitors()
{
    for (auto &hMonitor : monitors)
        DestroyPhysicalMonitor(hMonitor);
}

// Create tray menu
HMENU CreateTrayMenu()
{
    HMENU menu = CreatePopupMenu();
    InsertMenu(menu, -1, MF_BYPOSITION, ID_EXIT, "Exit");
    return menu;
}

// Window procedure for tray events
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TRAYICON:
        {
            if (lParam == WM_LBUTTONUP)
                CycleBrightness();

            if (lParam == WM_RBUTTONUP)
            {
                POINT p;
                GetCursorPos(&p);
                HMENU menu = CreateTrayMenu();
                SetForegroundWindow(hwnd);

                int cmd = TrackPopupMenu(
                    menu,
                    TPM_RETURNCMD | TPM_NONOTIFY,
                    p.x, p.y,
                    0,
                    hwnd,
                    NULL
                );

                if (cmd == ID_EXIT)
                    PostQuitMessage(0);

                DestroyMenu(menu);
            }
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    InitMonitors();

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "BrightnessTrayApp";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        "BrightnessTrayApp",
        "",
        WS_OVERLAPPEDWINDOW,
        0, 0, 0, 0,
        NULL, NULL,
        hInstance,
        NULL
    );

    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BRIGHTNESS));
    strcpy_s(nid.szTip, "Monitor Brightness");

    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    CleanupMonitors();

    return 0;
}
