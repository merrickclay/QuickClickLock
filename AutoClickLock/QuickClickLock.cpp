#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include <commctrl.h>

#define WIN_SIZE_X 320
#define WIN_SIZE_Y 240
#define QL_TOGGLE_HOTKEY_ID 0x01
#define CUSTOM_HOTKEY_CLASS_ID 0x01

HWND hwndMain;
HWND hwndHotCtrl;
HINSTANCE hInst;

// forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OwnerDrawButtonProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
HWND WINAPI InitializeHotkeyControl(HWND hwndDlg);
BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl);
void ToggleClickLock();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"MAIN WINDOW";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    hwndMain = CreateWindowEx(
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

    if (hwndMain == NULL)
    {
        return 0;
    }

    ShowWindow(hwndMain, nCmdShow);

    hInst = hInstance;

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            MessageBox(hwndMain, L"An issue occurred while trying to register the ClickLock Toggle hotkey.\nMake sure the hotkey isn't used elsewhere and reassign the hotkey in the Quick ClickLock main window.",
                L"Hotkey Error", MB_ICONERROR);
            if ((int)msg.wParam == QL_TOGGLE_HOTKEY_ID) {
                ToggleClickLock();
            }

        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CREATE:
        {
            hwndHotCtrl = InitializeHotkeyControl(hwnd);
            AssignHotkey(hwndMain, hwndHotCtrl);            
            break;
        }
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);



            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_LBUTTONDOWN:
        {
            SetFocus(hwnd);
            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY:
        {
            UnregisterHotKey(hwnd, QL_TOGGLE_HOTKEY_ID);
            PostQuitMessage(0);
            return 0;
            break;
        }
        default:
        {
            DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
        }
        return 0;
    }
}

LRESULT CALLBACK HotkeyControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
        case WM_KILLFOCUS:
        {
            AssignHotkey(hwndMain, hwndHotCtrl);
        }
        default:
        {
            DefWindowProc(hWnd, uMsg, wParam, lParam);
            DefSubclassProc(hWnd, uMsg, wParam, lParam);
            break;
        }
        return 0;
    }
}

HWND WINAPI InitializeHotkeyControl(HWND hwnd)
{

    HWND hwndHot = NULL;

    // Ensure that the common control DLL is loaded. 
    INITCOMMONCONTROLSEX icex;  //declare an INITCOMMONCONTROLSEX Structure
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_HOTKEY_CLASS;   //set dwICC member to ICC_HOTKEY_CLASS    
                                     // this loads the Hot Key control class.
    InitCommonControlsEx(&icex);

    hwndHot = CreateWindowEx(0,                        // no extended styles 
        HOTKEY_CLASS,             // class name 
        L"",                 // no title (caption) 
        WS_CHILD | WS_VISIBLE,    // style 
        10, 10,                   // position 
        150, 20,                  // size 
        hwnd,                  // parent window 
        NULL,                     // uses class menu 
        hInst,                  // instance 
        NULL);                    // no WM_CREATE parameter 

    SetWindowSubclass(hwndHot, HotkeyControlProc, CUSTOM_HOTKEY_CLASS_ID, NULL);

    SendMessage(hwndHot,
        HKM_SETRULES,
        (WPARAM)HKCOMB_NONE | HKCOMB_S,   // invalid key combinations 
        MAKELPARAM(HOTKEYF_CONTROL, 0));

// Set CTRL + ALT + A as the default hot key for this window. 
// 0x41 is the virtual key code for 'A'. 
    SendMessage(hwndHot,
        HKM_SETHOTKEY,
        MAKEWORD(0x4C, HOTKEYF_CONTROL | HOTKEYF_SHIFT),
        0);

    return hwndHot;
}

void ToggleClickLock() {
    BOOL clickLockEnabled;
    SystemParametersInfoW(SPI_GETMOUSECLICKLOCK, 0, &clickLockEnabled, 0);
    if (clickLockEnabled) {
        SystemParametersInfoW(SPI_SETMOUSECLICKLOCK, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
    }
    else {
        SystemParametersInfoW(SPI_SETMOUSECLICKLOCK, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
    }
}

BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl) {
    UnregisterHotKey(NULL, QL_TOGGLE_HOTKEY_ID);
    LRESULT hotkeyData = SendMessageW(hwndHotCtrl, HKM_GETHOTKEY, 0, 0);
    byte modifiers = HIBYTE(hotkeyData) | MOD_NOREPEAT;

    // Compensate for difference between HOTKEYF_SHIFT/HOTKEYF_ALT and MOD_SHIFT/MOD_ALT
    if (modifiers & 0x01 & ~0x04) {
        modifiers = modifiers & ~0x01 | 0x04;
    }
    else if(modifiers & 0x04) {
        modifiers = modifiers & ~0x04 | 0x01;
    }

    byte vKey = LOBYTE(hotkeyData);
    BOOL assignedHotkey = RegisterHotKey(NULL, QL_TOGGLE_HOTKEY_ID, modifiers, vKey);
    if (!assignedHotkey)
    {
        MessageBox(hwndMain, L"An issue occurred while trying to register the ClickLock Toggle hotkey.\nMake sure the hotkey isn't used elsewhere and reassign the hotkey in the Quick ClickLock main window.",
            L"Hotkey Error", MB_ICONERROR);
    }
    return assignedHotkey;
}