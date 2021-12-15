#ifndef UNICODE
#define UNICODE
#endif 

#include <string>
#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include <commctrl.h>
#include <shlObj.h>
#include <strsafe.h>
#include <shellapi.h>
#include "resource.h"

#define APP_NAME L"QUICK_CLICKLOCK"
#define WM_TRAYMSG (WM_USER + 1)
#define WIN_SIZE_X 320
#define WIN_SIZE_Y 160
#define CLICKLOCK_TIME_MIN 200
#define CLICKLOCK_TIME_MAX 2200
#define CLICKLOCK_TIME_DEFAULT 1200
#define QL_TOGGLE_HOTKEY_ID 0x01
#define CUSTOM_HOTKEY_CLASS_ID 0x01
#define MENU_SETTINGS_ID 0
#define MENU_EXIT_ID 1

static UINT WM_TASKBARCREATED;

const LPCTSTR APPDATA_FOLDER = L"QuickClickLock\\";
const LPCTSTR INI_FILENAME = L"quick_clicklock.ini";
const int DEFAULT_HOTKEY = 0x4C;
const int DEFAULT_MODIFIERS = HOTKEYF_CONTROL | HOTKEYF_SHIFT;
const int DEFAULT_ACTIVATION_TIME = 1200;

RECT rcClient;
HBRUSH hBrushLabel;
COLORREF clrLabelText;
COLORREF clrLabelBkGnd;

INITCOMMONCONTROLSEX icex;
HICON hIconGreenSmall = NULL;
HICON hIconGreen = NULL;
HICON hIconRedSmall = NULL;
HICON hIconRed = NULL;
NOTIFYICONDATA nidTrayIcon;
HWND hwndMain = NULL;
HWND hwndUpDnGroupBox = NULL;
HWND hwndHotkeyLabel = NULL;
HWND hwndHotCtrl = NULL;
HWND hwndTimeLabel = NULL;
HWND hwndUpDnEdtBdy = NULL;
HWND hwndUpDnCtl = NULL;
BOOL UpDnCtlInitialized = FALSE;
HINSTANCE hInst = NULL;

TCHAR iniFilePath[MAX_PATH];
struct IniVars {
    int activationTime;
    int shortcutKey;
    int shortcutModifiers;
} savedVars;

// forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HotkeyControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
INT_PTR CALLBACK UpDownDialogProc(HWND, UINT, WPARAM, LPARAM);
HWND CreateHotkeyControl(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateUpDownControl(HWND hwndParent);
HWND CreateUpDownBuddy(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateLabel(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
HWND CreateGroupBox(HWND hwndParent, LPCWSTR windowName, int x, int y, int width, int height);
NOTIFYICONDATA AddTrayIcon();
void SendNotification(const wchar_t* title, const wchar_t* message);
void RemoveTrayIcon(NOTIFYICONDATA nid);
BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl);
BOOL GetClickLockStatus();
void ToggleClickLock();
void PlayAudioNotif(BOOL clickLockEnabled);
void SetWindowIcons(BOOL clickLockEnabled, HWND hwnd, NOTIFYICONDATA nid);
HICON GetTrayIcon(BOOL clickLockEnabled);
void SetActivationTimer(int val);
void AssignDefaultVars();
BOOL FetchIniFilePath();
BOOL CreateIniFile();
void ReadIniFile();
BOOL UpdateIniFile();
BOOL DirectoryExists(LPCTSTR szPath);
BOOL FileExists(LPCTSTR szPath);


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Initialize savedVars
    AssignDefaultVars();
    if (FetchIniFilePath()) { // fetching path succeeded
        if (FileExists(iniFilePath)) { // ini file exists
            ReadIniFile();
        }
        else { // ini file doesn't exist

            if (CreateIniFile()) { //ini file created

                UpdateIniFile();
            }
        }
    }

    hInst = hInstance;

    // Load resources
    hIconGreenSmall = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GREEN_SMALL)); 
    hIconGreen = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GREEN_LARGE));
    hIconRedSmall = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RED_SMALL));
    hIconRed = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RED_LARGE));

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"MAIN WINDOW";

    WNDCLASSEX wcex = { };

    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = WindowProc;
    wcex.hInstance = hInstance;
    wcex.lpszClassName = CLASS_NAME;
    wcex.hIcon = hIconGreen;
    wcex.hIconSm = hIconGreenSmall;

    RegisterClassExW(&wcex);

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

    nidTrayIcon = AddTrayIcon();
    ShowWindow(hwndMain, SW_HIDE);

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
            WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));

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

            BOOL clickLockEnabled = GetClickLockStatus();
            PlayAudioNotif(clickLockEnabled);
            SetWindowIcons(clickLockEnabled, hwndMain, nidTrayIcon);
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
            if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == hwndUpDnEdtBdy && UpDnCtlInitialized) {
                BOOL failed;
                int result = SendMessage(hwndUpDnCtl, UDM_GETPOS32, 0, (LPARAM)&failed);
                if (failed) {
                    result = result >= CLICKLOCK_TIME_MAX ? CLICKLOCK_TIME_MAX : result <= CLICKLOCK_TIME_MIN ? CLICKLOCK_TIME_MIN : CLICKLOCK_TIME_DEFAULT;
                    SendMessage(hwndUpDnCtl, UDM_SETPOS, 0, result);
                }
                SetActivationTimer(result);
            }
            else if (HIWORD(wParam) == 0 && lParam == 0) {
                switch(LOWORD(wParam))
                {
                case MENU_SETTINGS_ID:
                    ShowWindow(hwndMain, SW_SHOW);
                    break;
                case MENU_EXIT_ID:
                    DestroyWindow(hwndMain);
                    break;
                default:
                    DefWindowProc(hwnd, uMsg, wParam, lParam);
                }
            }
            break;
        }
        case WM_TRAYMSG:
            switch (lParam)
            {
            case WM_LBUTTONDOWN:
            {
                ShowWindow(hwndMain, SW_SHOW);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                POINT mousePos;
                GetCursorPos(&mousePos);
                HMENU hPopupMenu = CreatePopupMenu();
                InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, MENU_SETTINGS_ID, L"Settings");
                InsertMenu(hPopupMenu, 1, MF_BYPOSITION | MF_STRING, MENU_EXIT_ID, L"Exit");
                SetForegroundWindow(hwndMain);
                TrackPopupMenu(hPopupMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, mousePos.x, mousePos.y, 0, hwnd, NULL);
                break;
            }
            default:
            {
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
            };
            break;
        case WM_CLOSE:
        {
            ShowWindow(hwndMain, SW_HIDE);
            SendNotification(L"Quick ClickLock has been minimized", L"Right click the system tray icon and select \"Exit\" to exit the program.");
            break;
        }
        case WM_DESTROY:
        {
            if (hBrushLabel) DeleteObject(hBrushLabel);
            UnregisterHotKey(hwnd, QL_TOGGLE_HOTKEY_ID);
            UpdateIniFile();
            RemoveTrayIcon(nidTrayIcon);
            PostQuitMessage(0);
            break;
        }
        default:
        {
            if (uMsg == WM_TASKBARCREATED)
                AddTrayIcon();
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

    SendMessage(hControl,
        HKM_SETHOTKEY,
        MAKEWORD(savedVars.shortcutKey, savedVars.shortcutModifiers),
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
    SendMessage(hControl, UDM_SETPOS, 0, (LPARAM)savedVars.activationTime);
    UpDnCtlInitialized = TRUE;

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

NOTIFYICONDATA AddTrayIcon() {
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwndMain;
    nid.uID = 1;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uCallbackMessage = WM_TRAYMSG;
    nid.hIcon = GetTrayIcon(GetClickLockStatus());
    wcscpy_s(nid.szTip, L"Quick ClickLock");
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid) ? S_OK : E_FAIL;
    return nid;
}

void SendNotification(const wchar_t* title, const wchar_t* message) {
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwndMain;
    nid.uID = 1;
    nid.uVersion = NOTIFYICON_VERSION_4;
    nid.uFlags = NIF_INFO;
    wcscpy_s(nid.szInfoTitle, title);
    wcscpy_s(nid.szInfo,  message);
    nid.dwInfoFlags = NIIF_NONE;

    Shell_NotifyIcon(NIM_MODIFY, &nid) ? S_OK : E_FAIL;
}

void RemoveTrayIcon(NOTIFYICONDATA nid) {
    Shell_NotifyIcon(NIM_DELETE, &nid) ? S_OK : E_FAIL;
}

BOOL AssignHotkey(HWND hwndMain, HWND hwndHotCtrl) {
    UnregisterHotKey(NULL, QL_TOGGLE_HOTKEY_ID);
    LRESULT hotkeyData = SendMessage(hwndHotCtrl, HKM_GETHOTKEY, 0, 0);
    byte modifiers = HIBYTE(hotkeyData);
    byte transformedMods = modifiers | MOD_NOREPEAT;

    // Compensate for difference between HOTKEYF_SHIFT/HOTKEYF_ALT and MOD_SHIFT/MOD_ALT
    if (transformedMods & 0x01 & ~0x04) {
        transformedMods = transformedMods & ~0x01 | 0x04;
    }
    else if(transformedMods & 0x04) {
        transformedMods = transformedMods & ~0x04 | 0x01;
    }

    byte vKey = LOBYTE(hotkeyData);
    BOOL assignedHotkey = RegisterHotKey(NULL, QL_TOGGLE_HOTKEY_ID, transformedMods, vKey);
    if (assignedHotkey)
    {
        savedVars.shortcutKey = vKey;
        savedVars.shortcutModifiers = modifiers;
    }
    else
    {
        MessageBox(hwndMain, L"An issue occurred while trying to register the ClickLock Toggle hotkey.\nMake sure the hotkey isn't used elsewhere and reassign the hotkey in the Quick ClickLock main window.",
            L"Hotkey Error", MB_ICONERROR);
    }
    return assignedHotkey;
}

BOOL GetClickLockStatus() {
    BOOL clickLockEnabled;
    SystemParametersInfo(SPI_GETMOUSECLICKLOCK, 0, &clickLockEnabled, 0);
    OutputDebugString(clickLockEnabled ? L"\nEnabled\n\n" : L"\nDisabled\n\n");
    return clickLockEnabled;
}

void ToggleClickLock() {
    BOOL clickLockEnabled = GetClickLockStatus();
    if (clickLockEnabled) {
        SystemParametersInfo(SPI_SETMOUSECLICKLOCK, 0, (PVOID)FALSE, SPIF_SENDCHANGE);
    }
    else {
        SystemParametersInfo(SPI_SETMOUSECLICKLOCK, 0, (PVOID)TRUE, SPIF_SENDCHANGE);
    }
    clickLockEnabled = !clickLockEnabled;
    PlayAudioNotif(clickLockEnabled);
    SetWindowIcons(clickLockEnabled, hwndMain, nidTrayIcon);
}

void PlayAudioNotif(BOOL clickLockEnabled) {
    if (clickLockEnabled) {
        PlaySoundW(MAKEINTRESOURCE(IDW_HIGH_NOTIF), hInst, SND_RESOURCE | SND_ASYNC);
    }
    else {
        PlaySoundW(MAKEINTRESOURCE(IDW_LOW_NOTIF), hInst, SND_RESOURCE | SND_ASYNC);
    }
}

void SetWindowIcons(BOOL clickLockEnabled, HWND hwnd, NOTIFYICONDATA nid) {
    if (clickLockEnabled) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconGreenSmall);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconGreen);
        nid.hIcon = hIconGreen;
    }
    else {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconRedSmall);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconRed);
        nid.hIcon = hIconRed;
    }
    Shell_NotifyIcon(NIM_MODIFY, &nid) ? S_OK : E_FAIL;
}

HICON GetTrayIcon(BOOL clickLockEnabled) {
    if (clickLockEnabled) {
        return hIconGreen;
    }
    return hIconRed;
}

void SetActivationTimer(int val) {
    if (val > CLICKLOCK_TIME_MAX) {
        val = CLICKLOCK_TIME_MAX;
    }
    else if (val < CLICKLOCK_TIME_MIN) {
        val = CLICKLOCK_TIME_MIN;
    }

    SystemParametersInfo(SPI_SETMOUSECLICKLOCKTIME, 0, (PVOID)val, SPIF_SENDCHANGE);
    savedVars.activationTime = val;
}

void AssignDefaultVars() {
    savedVars.activationTime = DEFAULT_ACTIVATION_TIME;
    savedVars.shortcutKey = DEFAULT_HOTKEY;
    savedVars.shortcutModifiers = DEFAULT_MODIFIERS;
}

BOOL FetchIniFilePath() {
    LPTSTR path;
    BOOL returnVal = TRUE;
    return SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path) +
        StringCchCat(iniFilePath, MAX_PATH, path) +
        StringCchCat(iniFilePath, MAX_PATH, L"\\") +
        StringCchCat(iniFilePath, MAX_PATH, APPDATA_FOLDER) +
        StringCchCat(iniFilePath, MAX_PATH, INI_FILENAME) == S_OK;
}

BOOL CreateIniFile() {
    LPTSTR directoryPath;
    
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &directoryPath) +
        StringCchCat(directoryPath, MAX_PATH, L"\\") +
        StringCchCat(directoryPath, MAX_PATH, APPDATA_FOLDER) != S_OK) {
        return FALSE;
    }
    if (!DirectoryExists(directoryPath)) {
        if (!CreateDirectory(directoryPath, NULL)) return FALSE;
    }

    HANDLE handle = CreateFile(iniFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
    else {
        DWORD error = GetLastError();
        if (error != ERROR_FILE_EXISTS) return FALSE;
    }

    return TRUE;
}

void ReadIniFile() {
    savedVars.activationTime = GetPrivateProfileInt(APP_NAME, L"ActivationTime", DEFAULT_ACTIVATION_TIME, iniFilePath);
    savedVars.shortcutKey = GetPrivateProfileInt(APP_NAME, L"ShortcutKey", DEFAULT_HOTKEY, iniFilePath);
    savedVars.shortcutModifiers = GetPrivateProfileInt(APP_NAME, L"ShortcutModifiers", DEFAULT_MODIFIERS, iniFilePath);
}

BOOL UpdateIniFile() {
    wchar_t timeStr[256];
    _itow_s(savedVars.activationTime, timeStr, 10);
    wchar_t keyStr[256];
    _itow_s(savedVars.shortcutKey, keyStr, 10);
    wchar_t modStr[256];
    _itow_s(savedVars.shortcutModifiers, modStr, 10);
    return WritePrivateProfileString(APP_NAME, L"ActivationTime", timeStr, iniFilePath) &
        WritePrivateProfileString(APP_NAME, L"ShortcutKey", keyStr, iniFilePath) &
        WritePrivateProfileString(APP_NAME, L"ShortcutModifiers", modStr, iniFilePath);
}

BOOL DirectoryExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL FileExists(LPCTSTR szPath)
{
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}