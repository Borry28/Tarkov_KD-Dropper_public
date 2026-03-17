// Tarkov_KD-Dropper_public.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "framework.h"
#include "Tarkov_KD-Dropper_public.h"
#include "utils.h"
#include "KdDropper.h"
#include <shellapi.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// UI Control Handles
HWND hBtnStart, hBtnStop, hBtnScavBot;
HWND hComboMap, hComboTime;
HWND hChkRandomMap, hChkRandomTime;
HWND hEditMaxDeaths, hEditStopKey, hEditGrenadeKey;
HWND hStaticStatus, hStaticLoopCount, hStaticDeathCount, hStaticAvgTime;
HWND hStaticLogo, hTooltip;

DWORD gStopKeybind = VK_ESCAPE;
DWORD gGrenadeKeybind = 'G';

HFONT hFontTitle, hFontSection, hFontBody, hFontValue, hFontButton, hFontCaption;
HBRUSH hBrushWindow, hBrushPanel, hBrushInput, hBrushButton;
HICON hLogoIcon;
bool hasCustomLogo = false;
long long gLastDisplayedAverageLoopSeconds = -1;

const COLORREF COLOR_BG = RGB(7, 13, 31);
const COLORREF COLOR_PANEL = RGB(12, 20, 44);
const COLORREF COLOR_INPUT = RGB(18, 28, 58);
const COLORREF COLOR_BORDER = RGB(34, 53, 102);
const COLORREF COLOR_TEXT = RGB(238, 242, 255);
const COLORREF COLOR_MUTED = RGB(142, 154, 188);
const COLORREF COLOR_ACCENT = RGB(100, 147, 255);
const COLORREF COLOR_ACCENT_ALT = RGB(161, 131, 255);
const COLORREF COLOR_SUCCESS = RGB(122, 225, 196);

#define TIMER_UPDATE_STATS 1

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                CreateControls(HWND hWnd);
void                UpdateStats();
void                CreateThemeResources();
void                DestroyThemeResources();
void                ApplyFont(HWND hWnd, HFONT hFont);
void                PaintWindowBackground(HWND hWnd, HDC hdc);
void                DrawButton(LPDRAWITEMSTRUCT drawItem);
void                LoadDefaultLogo();
void                SetLogoIcon(HICON icon, bool customIcon);
void                CreateTooltips(HWND hWnd);
void                AddTooltip(HWND tooltip, HWND target, UINT_PTR id, const wchar_t* text);
std::wstring        GetKeybindDisplayText(DWORD virtualKey);
void                RefreshKeybindText(HWND target);
bool                IsInputControl(HWND hWnd);
void                DeselectFocusedInput(HWND hWnd, LPARAM lParam);
LRESULT CALLBACK    KeybindEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR);

namespace
{
    bool IsExtendedVirtualKey(DWORD virtualKey)
    {
        switch (virtualKey)
        {
        case VK_RMENU:
        case VK_RCONTROL:
        case VK_INSERT:
        case VK_DELETE:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_NUMLOCK:
        case VK_CANCEL:
        case VK_SNAPSHOT:
        case VK_DIVIDE:
        case VK_LWIN:
        case VK_RWIN:
        case VK_APPS:
            return true;
        default:
            return false;
        }
    }

    Maps MapFromComboIndex(int comboIndex)
    {
        return comboIndex < 0
            ? Maps::Factory
            : static_cast<Maps>(comboIndex + static_cast<int>(Maps::Interchange));
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    INITCOMMONCONTROLSEX initCommonControls{};
    initCommonControls.dwSize = sizeof(initCommonControls);
    initCommonControls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&initCommonControls);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TARKOVKDDROPPERPUBLIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = nullptr;

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



bool IsInputControl(HWND hWnd)
{
    return hWnd == hEditStopKey
        || hWnd == hEditGrenadeKey
        || hWnd == hEditMaxDeaths
        || hWnd == hComboMap
        || hWnd == hComboTime;
}

void DeselectFocusedInput(HWND hWnd, LPARAM lParam)
{
    HWND focusedWindow = GetFocus();
    if (!IsInputControl(focusedWindow))
    {
        return;
    }

    POINT clickPoint = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    HWND clickedWindow = ChildWindowFromPointEx(hWnd, clickPoint, CWP_SKIPDISABLED | CWP_SKIPINVISIBLE);
    if (clickedWindow == nullptr)
    {
        clickedWindow = hWnd;
    }

    if (!IsInputControl(clickedWindow))
    {
        SetFocus(hWnd);
    }
}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TARKOVKDDROPPERPUBLIC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = nullptr;
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_TARKOVKDDROPPERPUBLIC));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, L"Tarkov KD Dropper", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, 0, 560, 470, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

    HICON windowIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_TARKOVKDDROPPERPUBLIC), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_SHARED);
    HICON windowIconSmall = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_TARKOVKDDROPPERPUBLIC), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED);
    SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)windowIcon);
    SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)windowIconSmall);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateThemeResources();
        CreateControls(hWnd);
        SetTimer(hWnd, TIMER_UPDATE_STATS, 100, NULL);
        break;
    case WM_TIMER:
        if (wParam == TIMER_UPDATE_STATS)
        {
            UpdateStats();
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        DeselectFocusedInput(hWnd, lParam);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDC_BTN_START:
                {
                    EnableWindow(hBtnStart, FALSE);
                    EnableWindow(hBtnStop, TRUE);
                    ShowWindow(hBtnStop, SW_SHOW);
                    std::thread([]() {
                        KdDropper::getInstance().start();
                    }).detach();
                }
                break;
            case IDC_BTN_STOP:
                {
                    KdDropper::getInstance().stop();
                    EnableWindow(hBtnStart, TRUE);
                    EnableWindow(hBtnStop, FALSE);
                    ShowWindow(hBtnStop, SW_HIDE);
                    ShowWindow(hBtnScavBot, SW_SHOW);
                }
                break;
            case IDC_BTN_SCAV_BOT:
                {
                    HINSTANCE openResult = ShellExecuteW(hWnd, L"open", L"https://tarkov.bot/products/scav-bot", nullptr, nullptr, SW_SHOWNORMAL);
                    if ((INT_PTR)openResult <= 32)
                    {
                        MessageBoxW(hWnd, L"Unable to open the Scav Bot page in your default browser.", L"Open Link Failed", MB_OK | MB_ICONWARNING);
                    }
                }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
        {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            SetBkMode(hdc, TRANSPARENT);
            if (hCtrl == hStaticStatus)
            {
                SetTextColor(hdc, KdDropper::getInstance().isRunning.load() ? COLOR_SUCCESS : COLOR_MUTED);
            }
            else if (hCtrl == hStaticLoopCount || hCtrl == hStaticDeathCount || hCtrl == hStaticAvgTime)
            {
                SetTextColor(hdc, COLOR_ACCENT);
            }
            else
            {
                SetTextColor(hdc, COLOR_TEXT);
            }
            return (LRESULT)hBrushPanel;
        }
    case WM_CTLCOLOREDIT:
        {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, COLOR_INPUT);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)hBrushInput;
        }
    case WM_CTLCOLORLISTBOX:
        {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, COLOR_INPUT);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)hBrushInput;
        }
    case WM_CTLCOLORBTN:
        {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, COLOR_PANEL);
            SetTextColor(hdc, COLOR_TEXT);
            return (LRESULT)hBrushPanel;
        }
    case WM_DRAWITEM:
        DrawButton((LPDRAWITEMSTRUCT)lParam);
        return TRUE;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            PaintWindowBackground(hWnd, hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        KillTimer(hWnd, TIMER_UPDATE_STATS);
        if (KdDropper::getInstance().isRunning)
        {
            KdDropper::getInstance().stop();
        }
        DestroyThemeResources();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CreateThemeResources()
{
    hFontTitle = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Bahnschrift SemiBold");
    hFontSection = CreateFontW(17, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI Semibold");
    hFontBody = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
    hFontValue = CreateFontW(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Bahnschrift SemiBold");
    hFontButton = CreateFontW(16, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI Semibold");
    hFontCaption = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

    hBrushWindow = CreateSolidBrush(COLOR_BG);
    hBrushPanel = CreateSolidBrush(COLOR_PANEL);
    hBrushInput = CreateSolidBrush(COLOR_INPUT);
    hBrushButton = CreateSolidBrush(COLOR_ACCENT);

    hLogoIcon = nullptr;
    hasCustomLogo = false;
}

void DestroyThemeResources()
{
    if (hasCustomLogo && hLogoIcon != nullptr)
    {
        DestroyIcon(hLogoIcon);
    }

    DeleteObject(hBrushWindow);
    DeleteObject(hBrushPanel);
    DeleteObject(hBrushInput);
    DeleteObject(hBrushButton);

    DeleteObject(hFontTitle);
    DeleteObject(hFontSection);
    DeleteObject(hFontBody);
    DeleteObject(hFontValue);
    DeleteObject(hFontButton);
    DeleteObject(hFontCaption);
}

void ApplyFont(HWND hWnd, HFONT hFont)
{
    SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void PaintPanel(HDC hdc, int left, int top, int right, int bottom)
{
    HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
    HGDIOBJ oldPen = SelectObject(hdc, hPen);
    HGDIOBJ oldBrush = SelectObject(hdc, hBrushPanel);
    RoundRect(hdc, left, top, right, bottom, 16, 16);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

void PaintWindowBackground(HWND hWnd, HDC hdc)
{
    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    FillRect(hdc, &clientRect, hBrushWindow);

    PaintPanel(hdc, 16, 16, 528, 110);
    PaintPanel(hdc, 16, 126, 264, 398);
    PaintPanel(hdc, 280, 126, 528, 286);
    PaintPanel(hdc, 280, 302, 528, 398);

    HPEN accentPen = CreatePen(PS_SOLID, 3, COLOR_ACCENT_ALT);
    HGDIOBJ oldPen = SelectObject(hdc, accentPen);
    MoveToEx(hdc, 34, 138, nullptr);
    LineTo(hdc, 110, 138);
    MoveToEx(hdc, 298, 138, nullptr);
    LineTo(hdc, 374, 138);
    MoveToEx(hdc, 298, 314, nullptr);
    LineTo(hdc, 374, 314);
    SelectObject(hdc, oldPen);
    DeleteObject(accentPen);
}

void DrawButton(LPDRAWITEMSTRUCT drawItem)
{
    if (drawItem == nullptr)
    {
        return;
    }

    COLORREF fillColor = COLOR_INPUT;
    COLORREF borderColor = COLOR_BORDER;
    COLORREF textColor = COLOR_TEXT;

    if (drawItem->CtlID == IDC_BTN_START)
    {
        fillColor = drawItem->itemState & ODS_DISABLED ? RGB(26, 37, 68) : RGB(22, 34, 70);
        borderColor = drawItem->itemState & ODS_DISABLED ? RGB(58, 74, 118) : RGB(96, 121, 198);
    }
    else if (drawItem->CtlID == IDC_BTN_STOP)
    {
        fillColor = drawItem->itemState & ODS_DISABLED ? RGB(52, 42, 88) : COLOR_ACCENT_ALT;
        borderColor = COLOR_ACCENT_ALT;
    }
    else if (drawItem->CtlID == IDC_BTN_SCAV_BOT)
    {
        fillColor = drawItem->itemState & ODS_DISABLED ? RGB(37, 55, 92) : COLOR_ACCENT;
        borderColor = COLOR_ACCENT;
    }

    HBRUSH fillBrush = CreateSolidBrush(fillColor);
    HPEN borderPen = CreatePen(PS_SOLID, 1, borderColor);
    HGDIOBJ oldBrush = SelectObject(drawItem->hDC, fillBrush);
    HGDIOBJ oldPen = SelectObject(drawItem->hDC, borderPen);

    RoundRect(drawItem->hDC,
        drawItem->rcItem.left,
        drawItem->rcItem.top,
        drawItem->rcItem.right,
        drawItem->rcItem.bottom,
        14,
        14);

    SelectObject(drawItem->hDC, oldBrush);
    SelectObject(drawItem->hDC, oldPen);

    SetBkMode(drawItem->hDC, TRANSPARENT);
    SetTextColor(drawItem->hDC, textColor);
    SelectObject(drawItem->hDC, hFontButton);

    WCHAR caption[128];
    GetWindowTextW(drawItem->hwndItem, caption, 128);
    DrawTextW(drawItem->hDC, caption, -1, &drawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if (drawItem->itemState & ODS_FOCUS)
    {
        RECT focusRect = drawItem->rcItem;
        InflateRect(&focusRect, -4, -4);
        DrawFocusRect(drawItem->hDC, &focusRect);
    }

    DeleteObject(fillBrush);
    DeleteObject(borderPen);
}

void SetLogoIcon(HICON icon, bool customIcon)
{
    if (hasCustomLogo && hLogoIcon != nullptr)
    {
        DestroyIcon(hLogoIcon);
    }

    hLogoIcon = icon;
    hasCustomLogo = customIcon;

    if (hStaticLogo != nullptr)
    {
        SendMessage(hStaticLogo, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hLogoIcon);
    }
}

void LoadDefaultLogo()
{
    HICON defaultIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_TARKOVKDDROPPERPUBLIC), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR | LR_SHARED);
    SetLogoIcon(defaultIcon, false);
}

std::wstring GetKeybindDisplayText(DWORD virtualKey)
{
    UINT scanCode = MapVirtualKeyW(virtualKey, MAPVK_VK_TO_VSC);
    LONG keyData = static_cast<LONG>(scanCode << 16);
    if (IsExtendedVirtualKey(virtualKey))
    {
        keyData |= 1 << 24;
    }

    wchar_t keyName[64]{};
    if (GetKeyNameTextW(keyData, keyName, _countof(keyName)) > 0)
    {
        return keyName;
    }

    if ((virtualKey >= '0' && virtualKey <= '9') || (virtualKey >= 'A' && virtualKey <= 'Z'))
    {
        return std::wstring(1, static_cast<wchar_t>(virtualKey));
    }

    wchar_t fallback[16]{};
    swprintf_s(fallback, L"VK %u", virtualKey);
    return fallback;
}

void RefreshKeybindText(HWND target)
{
    if (target == hEditStopKey)
    {
        const std::wstring keyText = GetKeybindDisplayText(gStopKeybind);
        SetWindowTextW(target, keyText.c_str());
    }
    else if (target == hEditGrenadeKey)
    {
        const std::wstring keyText = GetKeybindDisplayText(gGrenadeKeybind);
        SetWindowTextW(target, keyText.c_str());
    }
}

LRESULT CALLBACK KeybindEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR)
{
    switch (message)
    {
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS | DLGC_WANTARROWS;
    case WM_SETFOCUS:
        SetWindowTextW(hWnd, L"Press any key...");
        SendMessageW(hWnd, EM_SETSEL, 0, -1);
        break;
    case WM_KILLFOCUS:
        RefreshKeybindText(hWnd);
        break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        {
            const DWORD virtualKey = static_cast<DWORD>(wParam);
            if (hWnd == hEditStopKey)
            {
                gStopKeybind = virtualKey;
                KdDropper::getInstance().setStopKeybind(virtualKey);
            }
            else if (hWnd == hEditGrenadeKey)
            {
                gGrenadeKeybind = virtualKey;
                KdDropper::getInstance().setEquipGrenadeKeybind(virtualKey);
            }

            RefreshKeybindText(hWnd);
            SetFocus(GetParent(hWnd));
            return 0;
        }
    case WM_CHAR:
    case WM_SYSCHAR:
        return 0;
    }

    return DefSubclassProc(hWnd, message, wParam, lParam);
}

void AddTooltip(HWND tooltip, HWND target, UINT_PTR id, const wchar_t* text)
{
    if (tooltip == nullptr || target == nullptr)
    {
        return;
    }

    TOOLINFOW toolInfo{};
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.uFlags = TTF_SUBCLASS;
    toolInfo.hwnd = target;
    toolInfo.uId = id;
    GetClientRect(target, &toolInfo.rect);
    toolInfo.lpszText = const_cast<LPWSTR>(text);
    SendMessageW(tooltip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&toolInfo));
}

void CreateTooltips(HWND hWnd)
{
    hTooltip = CreateWindowExW(WS_EX_TOPMOST,
        TOOLTIPS_CLASS,
        nullptr,
        WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hWnd,
        nullptr,
        hInst,
        nullptr);

    if (hTooltip == nullptr)
    {
        return;
    }

    SetWindowPos(hTooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SendMessageW(hTooltip, TTM_ACTIVATE, TRUE, 0);
    SendMessageW(hTooltip, TTM_SETMAXTIPWIDTH, 0, 260);
    SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 300);
    SendMessageW(hTooltip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 12000);

    AddTooltip(hTooltip, hStaticLogo, 1, L"Application branding area. The app icon is fixed and no longer user-selectable.");
    AddTooltip(hTooltip, hComboMap, 2, L"Select the raid map the bot should queue into when random map selection is disabled.");
    AddTooltip(hTooltip, hComboTime, 3, L"Choose whether the bot should prefer day or night raids when random time is disabled.");
    AddTooltip(hTooltip, hChkRandomMap, 4, L"Override the selected map and let the bot choose a map at random for each run.");
    AddTooltip(hTooltip, hChkRandomTime, 5, L"Override the selected time of day and randomize between day and night raids.");
    AddTooltip(hTooltip, hEditMaxDeaths, 6, L"Maximum number of deaths before the bot stops. Use 0 to allow unlimited runs.");
    AddTooltip(hTooltip, hEditStopKey, 7, L"Emergency stop key watched while the bot is active. Press ESC in-game to interrupt the loop.");
    AddTooltip(hTooltip, hEditGrenadeKey, 8, L"The in-game hotkey used to equip the grenade for the death loop sequence.");
    AddTooltip(hTooltip, hBtnStart, 9, L"Start the automation loop using the current settings.");
    AddTooltip(hTooltip, hBtnStop, 10, L"Stop the automation loop and wait for the current activity to unwind safely.");
}

void CreateControls(HWND hWnd)
{
    HWND hStatic = CreateWindowW(L"STATIC", L"Tarkov KD Dropper", WS_VISIBLE | WS_CHILD | SS_LEFT,
        92, 24, 260, 32, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontTitle);

    hStatic = CreateWindowW(L"STATIC", L"Instructions: Open Tarkov, empty your inventory and put one grenade in your inventory set your keybinds and config, press start, enjoy :)\r\nPlease check out our Scav Bot for more advanced automation", WS_VISIBLE | WS_CHILD | SS_LEFT,
        92, 56, 360, 42, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStaticLogo = CreateWindowW(L"STATIC", NULL, WS_VISIBLE | WS_CHILD | SS_ICON,
        30, 30, 48, 48, hWnd, NULL, hInst, NULL);

    hStatic = CreateWindowW(L"STATIC", L"Configuration", WS_VISIBLE | WS_CHILD | SS_LEFT,
        32, 146, 140, 20, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontSection);

    hStatic = CreateWindowW(L"STATIC", L"Map", WS_VISIBLE | WS_CHILD | SS_LEFT,
        32, 176, 80, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hComboMap = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        32, 196, 208, 220, hWnd, (HMENU)IDC_COMBO_MAP, hInst, NULL);
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Interchange");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Factory");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Shoreline");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Customs");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"LightHouse");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Streets Of Tarkov");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Woods");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Reserve");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Ground Zero");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Labs");
    SendMessage(hComboMap, CB_ADDSTRING, 0, (LPARAM)L"Terminal");
    SendMessage(hComboMap, CB_SETCURSEL, 1, 0);
    ApplyFont(hComboMap, hFontBody);

    hStatic = CreateWindowW(L"STATIC", L"Time of Day", WS_VISIBLE | WS_CHILD | SS_LEFT,
        32, 228, 90, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hComboTime = CreateWindowW(L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
        32, 248, 208, 100, hWnd, (HMENU)IDC_COMBO_TIME, hInst, NULL);
    SendMessage(hComboTime, CB_ADDSTRING, 0, (LPARAM)L"Day");
    SendMessage(hComboTime, CB_ADDSTRING, 0, (LPARAM)L"Night");
    SendMessage(hComboTime, CB_SETCURSEL, 0, 0);
    ApplyFont(hComboTime, hFontBody);

    hChkRandomMap = CreateWindowW(L"BUTTON", L"Use random map", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
        32, 278, 140, 20, hWnd, (HMENU)IDC_CHK_RANDOM_MAP, hInst, NULL);
    ApplyFont(hChkRandomMap, hFontCaption);

    hChkRandomTime = CreateWindowW(L"BUTTON", L"Use random time", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | WS_TABSTOP,
        32, 302, 140, 20, hWnd, (HMENU)IDC_CHK_RANDOM_TIME, hInst, NULL);
    ApplyFont(hChkRandomTime, hFontCaption);

    hStatic = CreateWindowW(L"STATIC", L"Stop", WS_VISIBLE | WS_CHILD | SS_LEFT,
        32, 334, 48, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStatic = CreateWindowW(L"STATIC", L"Grenade", WS_VISIBLE | WS_CHILD | SS_LEFT,
        108, 334, 58, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStatic = CreateWindowW(L"STATIC", L"Max Deaths", WS_VISIBLE | WS_CHILD | SS_LEFT,
        184, 334, 60, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hEditStopKey = CreateWindowW(L"EDIT", L"ESC", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER,
        32, 352, 64, 24, hWnd, (HMENU)IDC_EDIT_STOP_KEY, hInst, NULL);
    ApplyFont(hEditStopKey, hFontBody);
    SetWindowSubclass(hEditStopKey, KeybindEditProc, IDC_EDIT_STOP_KEY, 0);
    RefreshKeybindText(hEditStopKey);

    hEditGrenadeKey = CreateWindowW(L"EDIT", L"G", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER,
        108, 352, 64, 24, hWnd, (HMENU)IDC_EDIT_GRENADE_KEY, hInst, NULL);
    ApplyFont(hEditGrenadeKey, hFontBody);
    SetWindowSubclass(hEditGrenadeKey, KeybindEditProc, IDC_EDIT_GRENADE_KEY, 0);
    RefreshKeybindText(hEditGrenadeKey);

    hEditMaxDeaths = CreateWindowW(L"EDIT", L"0", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_CENTER,
        184, 352, 56, 24, hWnd, (HMENU)IDC_EDIT_MAX_DEATHS, hInst, NULL);
    ApplyFont(hEditMaxDeaths, hFontBody);

    hStatic = CreateWindowW(L"STATIC", L"Live Stats", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 146, 140, 20, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontSection);

    hStatic = CreateWindowW(L"STATIC", L"Status", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 176, 80, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStaticStatus = CreateWindowW(L"STATIC", L"Idle", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 198, 190, 24, hWnd, (HMENU)IDC_STATIC_STATUS, hInst, NULL);
    ApplyFont(hStaticStatus, hFontValue);

    hStatic = CreateWindowW(L"STATIC", L"Loop Count", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 236, 90, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStaticLoopCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 258, 90, 24, hWnd, (HMENU)IDC_STATIC_LOOP_COUNT, hInst, NULL);
    ApplyFont(hStaticLoopCount, hFontValue);

    hStatic = CreateWindowW(L"STATIC", L"Deaths", WS_VISIBLE | WS_CHILD | SS_LEFT,
        376, 236, 72, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStaticDeathCount = CreateWindowW(L"STATIC", L"0", WS_VISIBLE | WS_CHILD | SS_LEFT,
        376, 258, 72, 24, hWnd, (HMENU)IDC_STATIC_DEATH_COUNT, hInst, NULL);
    ApplyFont(hStaticDeathCount, hFontValue);

    hStatic = CreateWindowW(L"STATIC", L"Avg Time", WS_VISIBLE | WS_CHILD | SS_LEFT,
        448, 236, 70, 18, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontCaption);

    hStaticAvgTime = CreateWindowW(L"STATIC", L"0 s", WS_VISIBLE | WS_CHILD | SS_LEFT,
        448, 258, 72, 24, hWnd, (HMENU)IDC_STATIC_AVG_TIME, hInst, NULL);
    ApplyFont(hStaticAvgTime, hFontValue);

    hStatic = CreateWindowW(L"STATIC", L"Actions", WS_VISIBLE | WS_CHILD | SS_LEFT,
        296, 322, 120, 20, hWnd, NULL, hInst, NULL);
    ApplyFont(hStatic, hFontSection);

    hBtnScavBot = CreateWindowW(L"BUTTON", L"Scav Bot", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | WS_TABSTOP,
        296, 360, 100, 30, hWnd, (HMENU)IDC_BTN_SCAV_BOT, hInst, NULL);
    ApplyFont(hBtnScavBot, hFontButton);

    hBtnStart = CreateWindowW(L"BUTTON", L"Start Bot", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | WS_TABSTOP,
        408, 360, 100, 30, hWnd, (HMENU)IDC_BTN_START, hInst, NULL);
    ApplyFont(hBtnStart, hFontButton);

    hBtnStop = CreateWindowW(L"BUTTON", L"Stop Bot", WS_VISIBLE | WS_CHILD | BS_OWNERDRAW | WS_TABSTOP,
        408, 360, 100, 30, hWnd, (HMENU)IDC_BTN_STOP, hInst, NULL);
    ApplyFont(hBtnStop, hFontButton);
    EnableWindow(hBtnStop, FALSE);
    ShowWindow(hBtnStop, SW_HIDE);

    LoadDefaultLogo();
    CreateTooltips(hWnd);
}

void UpdateStats()
{
    auto& dropper = KdDropper::getInstance();
    const bool isRunning = dropper.isRunning.load();
    const bool useRandomMap = SendMessage(hChkRandomMap, BM_GETCHECK, 0, 0) == BST_CHECKED;
    const bool useRandomTime = SendMessage(hChkRandomTime, BM_GETCHECK, 0, 0) == BST_CHECKED;

    EnableWindow(hBtnStart, !isRunning);
    EnableWindow(hBtnStop, isRunning);
    ShowWindow(hBtnStop, isRunning ? SW_SHOW : SW_HIDE);
    ShowWindow(hBtnScavBot, isRunning ? SW_HIDE : SW_SHOW);
    EnableWindow(hComboMap, !useRandomMap);
    EnableWindow(hComboTime, !useRandomTime);

    std::string statusText = dropper.getStatus();
    std::wstring status(statusText.begin(), statusText.end());
    SetWindowTextW(hStaticStatus, status.c_str());

    std::wstring loopCount = std::to_wstring(dropper.getLoopCount());
    SetWindowTextW(hStaticLoopCount, loopCount.c_str());

    std::wstring deathCount = std::to_wstring(dropper.getDeathCount());
    SetWindowTextW(hStaticDeathCount, deathCount.c_str());

    const float averageLoopTimeMs = dropper.getAverageLoopTime();
    const long long averageLoopTimeSeconds = static_cast<long long>((averageLoopTimeMs + 500.0f) / 1000.0f);
    if (averageLoopTimeSeconds != gLastDisplayedAverageLoopSeconds)
    {
        wchar_t avgTime[32];
        swprintf_s(avgTime, L"%lld s", averageLoopTimeSeconds);
        SetWindowTextW(hStaticAvgTime, avgTime);
        gLastDisplayedAverageLoopSeconds = averageLoopTimeSeconds;
    }

    if (!isRunning)
    {
        int mapIndex = (int)SendMessage(hComboMap, CB_GETCURSEL, 0, 0);
        dropper.setMap(MapFromComboIndex(mapIndex));

        int timeIndex = (int)SendMessage(hComboTime, CB_GETCURSEL, 0, 0);
        dropper.setTimeOfDay((Time)timeIndex);

        dropper.setUseRandomMap(useRandomMap);
        dropper.setUseRandomTimeOfDay(useRandomTime);
        dropper.setStopKeybind(gStopKeybind);
        dropper.setEquipGrenadeKeybind(gGrenadeKeybind);

        wchar_t maxDeathsStr[16];
        GetWindowTextW(hEditMaxDeaths, maxDeathsStr, 16);
        dropper.setMaxDeaths(_wtoi(maxDeathsStr));
    }
}
