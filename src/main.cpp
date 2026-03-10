#include <windows.h>
#include <shellapi.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <vector>

#pragma comment(lib,"Dxva2.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_EXIT 1001

std::vector<PHYSICAL_MONITOR> monitors;

int levels[] = {0,20,40,60,80,100};
int levelIndex = 0;

void SetBrightnessAll(int value)
{
    for(auto &m : monitors)
        SetMonitorBrightness(m.hPhysicalMonitor,value);
}

void CycleBrightness()
{
    levelIndex++;
    if(levelIndex >= 6) levelIndex = 0;

    SetBrightnessAll(levels[levelIndex]);
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMon,HDC,LPRECT,LPARAM)
{
    DWORD count = 0;

    if(!GetNumberOfPhysicalMonitorsFromHMONITOR(hMon,&count))
        return TRUE;

    std::vector<PHYSICAL_MONITOR> temp(count);

    if(GetPhysicalMonitorsFromHMONITOR(hMon,count,temp.data()))
    {
        for(auto &m : temp)
            monitors.push_back(m);
    }

    return TRUE;
}

void InitMonitors()
{
    EnumDisplayMonitors(NULL,NULL,MonitorEnumProc,NULL);
}

void CleanupMonitors()
{
    for(auto &m : monitors)
        DestroyPhysicalMonitor(m.hPhysicalMonitor);
}

HMENU CreateTrayMenu()
{
    HMENU menu = CreatePopupMenu();
    InsertMenu(menu,-1,MF_BYPOSITION,ID_EXIT,"Exit");
    return menu;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
    switch(msg)
    {
        case WM_TRAYICON:
        {
            if(lParam == WM_LBUTTONUP)
            {
                CycleBrightness();
            }

            if(lParam == WM_RBUTTONUP)
            {
                POINT p;
                GetCursorPos(&p);

                HMENU menu = CreateTrayMenu();
                SetForegroundWindow(hwnd);

                int cmd = TrackPopupMenu(
                    menu,
                    TPM_RETURNCMD | TPM_NONOTIFY,
                    p.x,p.y,
                    0,
                    hwnd,
                    NULL
                );

                if(cmd == ID_EXIT)
                    PostQuitMessage(0);

                DestroyMenu(menu);
            }
        }
        break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd,msg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int)
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
        0,0,0,0,
        NULL,NULL,
        hInstance,
        NULL
    );

    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = (HICON)LoadImage(
        NULL,
        "../resources/icons/brightness.ico",
        IMAGE_ICON,
        16,
        16,
        LR_LOADFROMFILE
    );

    strcpy_s(nid.szTip,"Monitor Brightness");

    Shell_NotifyIcon(NIM_ADD,&nid);

    MSG msg;
    while(GetMessage(&msg,NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE,&nid);

    CleanupMonitors();

    return 0;
}
