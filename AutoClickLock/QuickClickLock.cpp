#ifndef UNICODE
#define UNICODE
#endif 

#define IDC_BUTTON 1001

#include <windows.h>
#include <winuser.h>

const int WIN_SIZE_X = 320;
const int WIN_SIZE_Y = 240;
HWND hButton;
HWND hMainWindow;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Quick ClickLock";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    hMainWindow = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Quick ClickLock",    // Window text
        WS_CAPTION | WS_SYSMENU,            // Window style

        // Size and position
        (GetSystemMetrics(SM_CXSCREEN) - WIN_SIZE_X) / 2, (GetSystemMetrics(SM_CYSCREEN) - WIN_SIZE_Y) / 2, WIN_SIZE_X, WIN_SIZE_Y,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hMainWindow == NULL)
    {
        return 0;
    }

    ShowWindow(hMainWindow, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_CREATE:
    {
        hButton = CreateWindowEx(0, L"BUTTON", L"Toggle",
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,
            0, 0, 50, 25,
            hwnd,
            (HMENU)IDC_BUTTON, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);



        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    case WM_COMMAND:
    {
        if (LOWORD(wParam == IDC_BUTTON)) {
            BOOL clickLockEnabled;
            SystemParametersInfoW(SPI_GETMOUSECLICKLOCK, 0, &clickLockEnabled, 0);
            if (clickLockEnabled) {
                SystemParametersInfoW(SPI_SETMOUSECLICKLOCK, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
                MessageBox(hMainWindow, L"ClickLock has been disabled", L"ClickLock Status", MB_ICONINFORMATION);
            }
            else {
                SystemParametersInfoW(SPI_SETMOUSECLICKLOCK, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
                MessageBox(hMainWindow, L"ClickLock has been enabled", L"ClickLock Status", MB_ICONINFORMATION);
            }

        }
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}