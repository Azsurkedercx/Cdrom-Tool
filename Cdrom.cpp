// cdrom.cpp
// 功能：两个按钮，弹出/关闭光驱
// 编译：i686-w64-mingw32-g++ -o cdrom.exe cdrom.cpp icon.o -mwindows -static -Os -s -Wl,--subsystem,windows:5.01

#include <windows.h>
#include <winioctl.h>

#define ID_BTN_EJECT 1001
#define ID_BTN_CLOSE 1002
#define IDI_MAIN_ICON 100

// 获取系统消息字体
static HFONT GetUIFont()
{
    NONCLIENTMETRICSW ncm;
    ncm.cbSize = sizeof(NONCLIENTMETRICSW);
    if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &ncm, 0))
    {
        return CreateFontIndirectW(&ncm.lfMessageFont);
    }
    return (HFONT)GetStockObject(DEFAULT_GUI_FONT);
}

// 弹出光驱
BOOL EjectCdRom()
{
    HANDLE hDevice = CreateFileW(
        L"\\\\.\\CdRom0",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_STORAGE_EJECT_MEDIA,
        NULL, 0,
        NULL, 0,
        &bytesReturned,
        NULL
    );

    CloseHandle(hDevice);
    return result;
}

// 关闭光驱
BOOL CloseCdRom()
{
    HANDLE hDevice = CreateFileW(
        L"\\\\.\\CdRom0",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    if (hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    DWORD bytesReturned = 0;
    BOOL result = DeviceIoControl(
        hDevice,
        IOCTL_STORAGE_LOAD_MEDIA,
        NULL, 0,
        NULL, 0,
        &bytesReturned,
        NULL
    );

    CloseHandle(hDevice);
    return result;
}

// 窗口消息处理
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HFONT hFont = GetUIFont();

        HWND hEject = CreateWindowW(L"BUTTON", L"弹出光驱",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            25, 18, 90, 30,
            hwnd, (HMENU)ID_BTN_EJECT,
            ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        HWND hClose = CreateWindowW(L"BUTTON", L"关闭光驱",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            25, 63, 90, 30,
            hwnd, (HMENU)ID_BTN_CLOSE,
            ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        if (hFont)
        {
            SendMessageW(hEject, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageW(hClose, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BTN_EJECT:
            EjectCdRom();
            return 0;
        case ID_BTN_CLOSE:
            CloseCdRom();
            return 0;
        }
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// 程序入口
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    // 加载图标（大/小各一份）
    HICON hIconBig = (HICON)LoadImageW(hInstance,
        MAKEINTRESOURCEW(IDI_MAIN_ICON),
        IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImageW(hInstance,
        MAKEINTRESOURCEW(IDI_MAIN_ICON),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    // 注册窗口类
    WNDCLASSEXW wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"CdRomToolClass";
    wc.hIcon         = hIconBig;
    wc.hIconSm       = hIconSmall;
    if (!RegisterClassExW(&wc))
        return 0;

    // 客户区 140x110，算含标题栏边框的整体尺寸
    RECT rc = {0, 0, 140, 110};
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    AdjustWindowRect(&rc, style, FALSE);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    // 计算屏幕居中位置
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - w) / 2;
    int y = (screenH - h) / 2;

    // 创建窗口（用计算好的位置）
    HWND hwnd = CreateWindowExW(
        0,
        L"CdRomToolClass",
        L"光驱控制",
        style,
        x, y, w, h,
        NULL, NULL, hInstance, NULL
    );
    if (!hwnd)
        return 0;

    // --- 关键：创建后再次显式设置大小图标（XP 下必须这步）---
    if (hIconBig)
        SendMessageW(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    if (hIconSmall)
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}