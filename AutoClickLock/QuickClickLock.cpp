#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include <commctrl.h>
#include <shlObj.h>

#define WIN_SIZE_X 320
#define WIN_SIZE_Y 160
#define CLICKLOCK_TIME_MIN 200
#define CLICKLOCK_TIME_MAX 2200
#define CLICKLOCK_TIME_DEFAULT 1200
#define QL_TOGGLE_HOTKEY_ID 0x01
#define CUSTOM_HOTKEY_CLASS_ID 0x01

RECT rcClient;
HBRUSH hBrushLabel;
COLORREF clrLabelText;
COLORREF clrLabelBkGnd;

INITCOMMONCONTROLSEX icex;
HWND hwndMain = NULL;
HWND hwndUpDnGroupBox = NULL;
HWND hwndHotkeyLabel = NULL;
HWND hwndHotCtrl = NULL;
HWND hwndTimeLabel = NULL;
HWND hwndUpDnEdtBdy = NULL;
HWND hwndUpDnCtl = NULL;
HINSTANCE hInst = NULL;

// forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HotkeyControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK UpDownDialogProc(HWND, UINT, WPARAM, LPARAM);
HWND CreateHotkeyControl(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateUpDownControl(HWND hwndParent);
HWND CreateUpDownBuddy(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateLabel(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateGroupBox(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl);
void ToggleClickLock();
void SetActivationTimer(int val);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);

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
        WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN,            // Window style

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
            GetClientRect(hwnd, &rcClient);
            hBrushLabel = NULL;
            clrLabelText = GetSysColor(COLOR_WINDOWTEXT);
            clrLabelBkGnd = GetSysColor(COLOR_WINDOW);

            hwndUpDnGroupBox = CreateGroupBox(hwnd, L"Settings", rcClient.left + 10, rcClient.top, rcClient.right - 20, rcClient.bottom - 10);
            hwndHotkeyLabel = CreateLabel(hwnd, L"Hotkey:", rcClient.left + 20, rcClient.top + 26, 55, 23);
            hwndHotCtrl = CreateHotkeyControl(hwnd, NULL, rcClient.left + 85, rcClient.top + 25, rcClient.right - 115, 23);
            hwndHotkeyLabel = CreateLabel(hwnd, L"Activation Time (ms):", rcClient.left + 20, rcClient.top + 76, 140, 23);
            hwndUpDnEdtBdy = CreateUpDownBuddy(hwnd, NULL, rcClient.left + 170, rcClient.top + 75, rcClient.right - 195, 23);
            hwndUpDnCtl = CreateUpDownControl(hwnd);
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
        case WM_CTLCOLORSTATIC:
        {
            HDC hdc = GetDC((HWND)wParam);
            SetTextColor(hdc, clrLabelText);
            SetBkColor(hdc, clrLabelBkGnd);
            if (!hBrushLabel) hBrushLabel = CreateSolidBrush(clrLabelBkGnd);
            return (LRESULT)hBrushLabel;
        }
        case WM_LBUTTONDOWN:
        {
            SetFocus(hwnd);
            break;
        }
        case WM_COMMAND:
        {
            if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == hwndUpDnEdtBdy) {
                BOOL failed;
                int result = SendMessage(hwndUpDnCtl, UDM_GETPOS32, 0, (LPARAM)&failed);
                if (failed) {
                    result = result >= CLICKLOCK_TIME_MAX ? CLICKLOCK_TIME_MAX : result <= CLICKLOCK_TIME_MIN ? CLICKLOCK_TIME_MIN : CLICKLOCK_TIME_DEFAULT;
                    SendMessage(hwndUpDnCtl, UDM_SETPOS, 0, result);
                }
                SetActivationTimer(result);
            }
            break;
        }
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
            break;
        }
        case WM_DESTROY:
        {
            if (hBrushLabel) DeleteObject(hBrushLabel);
            UnregisterHotKey(hwnd, QL_TOGGLE_HOTKEY_ID);
            PostQuitMessage(0);
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

HWND CreateHotkeyControl(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height)
{

    icex.dwICC = ICC_HOTKEY_CLASS;
    InitCommonControlsEx(&icex);

    HWND hControl = CreateWindowEx(0,                        // no extended styles 
        HOTKEY_CLASS,             // class name 
        windowName,
        WS_CHILD | WS_VISIBLE,    // style 
        x, y,                   // position 
        width, height,                  // size 
        hwndParent,                  // parent window 
        NULL,                     // uses class menu 
        hInst,                  // instance 
        NULL);                    // no WM_CREATE parameter 

    SetWindowSubclass(hControl, HotkeyControlProc, CUSTOM_HOTKEY_CLASS_ID, NULL);

    SendMessage(hControl,
        HKM_SETRULES,
        (WPARAM)HKCOMB_NONE | HKCOMB_S,   // invalid key combinations 
        MAKELPARAM(HOTKEYF_CONTROL, 0)); // substitution for invalid combinations

    // Set CTRL + SHIFT + L as the default hot key for this window. 
    // 0x4C is the virtual key code for 'L'. 
    SendMessage(hControl,
        HKM_SETHOTKEY,
        MAKEWORD(0x4C, HOTKEYF_CONTROL | HOTKEYF_SHIFT),
        0);

    return hControl;
}

HWND CreateUpDownControl(HWND hwndParent) {
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    HWND hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
        UPDOWN_CLASS,
        NULL,
        WS_CHILDWINDOW | WS_VISIBLE
        | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK | UDS_NOTHOUSANDS,
        0, 0,
        0, 0,         // Set to zero to automatically size to fit the buddy window.
        hwndParent,
        NULL,
        hInst,
        NULL);

    SendMessage(hControl, UDM_SETRANGE, 0, MAKELPARAM(CLICKLOCK_TIME_MAX, CLICKLOCK_TIME_MIN));
    SendMessage(hControl, UDM_SETPOS, 0, CLICKLOCK_TIME_DEFAULT);

    return (hControl);
}

HWND CreateUpDownBuddy(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height) {
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    HWND hControl = CreateWindowExW(WS_EX_LEFT | WS_EX_CLIENTEDGE | WS_EX_CONTEXTHELP,    //Extended window styles.
        WC_EDIT,
        windowName,
        WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER    // Window styles.
        | ES_NUMBER | ES_LEFT,                     // Edit control styles.
        x, y,
        width, height,
        hwndParent,
        NULL,
        hInst,
        NULL);

    return (hControl);
}

HWND CreateLabel(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height)
{
    HWND hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_LTRREADING,
        WC_STATIC,
        windowName,
        WS_CHILDWINDOW | WS_VISIBLE | SS_RIGHT,
        x, y,
        width, height,
        hwndParent,
        NULL,
        hInst,
        NULL);

    return (hControl);
}

HWND CreateGroupBox(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height)
{
    HWND hControl = CreateWindowEx(WS_EX_LEFT | WS_EX_CONTROLPARENT | WS_EX_LTRREADING,
        WC_BUTTON,
        windowName,
        WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE | WS_GROUP | BS_GROUPBOX | WS_TABSTOP,
        x, y,
        width, height,
        hwndParent,
        NULL,
        hInst,
        NULL);

    return (hControl);
}

BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl) {
    UnregisterHotKey(NULL, QL_TOGGLE_HOTKEY_ID);
    LRESULT hotkeyData = SendMessage(hwndHotCtrl, HKM_GETHOTKEY, 0, 0);
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

void ToggleClickLock() {
    BOOL clickLockEnabled;
    SystemParametersInfo(SPI_GETMOUSECLICKLOCK, 0, &clickLockEnabled, 0);
    if (clickLockEnabled) {
        if (SystemParametersInfo(SPI_SETMOUSECLICKLOCK, 0, (PVOID)FALSE, SPIF_SENDCHANGE)) {
            // play sound
        }
    }
    else {
        if (SystemParametersInfo(SPI_SETMOUSECLICKLOCK, 0, (PVOID)TRUE, SPIF_SENDCHANGE)) {
            // play sound
        }
    }
}

void SetActivationTimer(int val) {
    if (val > CLICKLOCK_TIME_MAX) {
        val = CLICKLOCK_TIME_MAX;
    }
    else if (val < CLICKLOCK_TIME_MIN) {
        val = CLICKLOCK_TIME_MIN;
    }

    SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 0, (PVOID)val, SPIF_SENDCHANGE);
}
