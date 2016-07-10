#include "Var.h"
#include "rc\resource.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#ifdef _DEBUG
#undef __CRT__NO_INLINE
#endif
#include <strsafe.h>
#ifdef _DEBUG
#define __CRT__NO_INLINE
#endif

#include "Inject.h"
#include "UI.h"
#include "Host.h"
#include "BasicIO.h"

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern HINSTANCE g_hInst;
// extern HWND g_hWnd;
extern int g_state;
extern WCHAR g_dllFullPath[MAX_PATH];
extern WCHAR g_dllName[MAX_PATH];
extern JV_ARG g_arg;

int g_dpiX = 0;
int g_dpiY = 0;

HWND JVUI_InitWindow(HINSTANCE hInstance)
{
	HWND hWnd;
	WNDCLASSEXW WndClsEx;

	// Get System DPI (System DPI Awareness)
	JVUI_GetSystemDPI();

	// Create Window
	ZeroMemory(&WndClsEx, sizeof(WNDCLASSEXW));
	WndClsEx.cbSize 		= sizeof(WNDCLASSEXW);
	WndClsEx.style 			= 0x0;
	WndClsEx.lpfnWndProc	= WndProcedure;
	WndClsEx.cbClsExtra 	= 0;
	WndClsEx.cbWndExtra 	= 0;
	WndClsEx.hIcon      	= (HICON) LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAINICON), IMAGE_ICON, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	WndClsEx.hCursor     	= (HCURSOR) LoadImageW(NULL, MAKEINTRESOURCEW(OCR_NORMAL), IMAGE_CURSOR, GetSystemMetrics(SM_CXCURSOR), GetSystemMetrics(SM_CYCURSOR), LR_SHARED);
	WndClsEx.hbrBackground 	= GetStockObject(WHITE_BRUSH);
	WndClsEx.lpszMenuName  	= NULL;
	WndClsEx.lpszClassName 	= JV_CLASS_NAME;
	WndClsEx.hInstance 		= hInstance;
	WndClsEx.hIconSm      	= (HICON) LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAINICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	if (WndClsEx.hIcon == NULL || WndClsEx.hIconSm == NULL)
	{
		fprintf(stderr, "LoadIcon() failed\nError Code : %lu\n\n", GetLastError());
		exit(1);
	}
	if (WndClsEx.hCursor == NULL)
	{
		fprintf(stderr, "LoadCursor() failed\nError Code : %lu\n\n", GetLastError());
		exit(1);
	}
	RegisterClassExW(&WndClsEx);

	hWnd = CreateWindowExW(WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT, // dwExStyle 0x080800A8
						JV_CLASS_NAME, 	// lpClassName
						JV_WINDOW_NAME, 	// lpWindowName
						WS_POPUP,		 // dwStyle, 0x80000000
						0, 		// X
						0, 		// Y
						0, 		// nWidth
						0, 		// nHeight
						NULL,		// hWndParent
						NULL, 		// hMenu
						hInstance,	// hInstance
						NULL);		// lpParam

	if (!IsWindow(hWnd)) // Is Window Created?
	{
		fprintf(stderr, "[ERR] CreateWindowEx() failed\nError Code : %lu\n\n", GetLastError());
		exit(1);
	}

	ShowWindow(hWnd, SW_SHOWNOACTIVATE);
	if (UpdateWindow(hWnd) == 0)
	{
		fprintf(stderr, "[ERR] UpdateWindow() failed\n\n");
		exit(1);
	}

	return hWnd = NULL;
}

void JVUI_GetSystemDPI()
{
	// Scale 100 == DPI 96
	// Scale 125 == DPI 120
	// Scale 150 == DPI 144
	// Scale 175 == DPI 168
	// Scale 200 == DPI 192

	// Get native resolution
	HDC desktopDC;

	desktopDC = GetDC(NULL);
	g_dpiX = GetDeviceCaps(desktopDC, LOGPIXELSX);
	g_dpiY = GetDeviceCaps(desktopDC, LOGPIXELSY);
}

// Right click BatteryLine icon in Notification Area
BOOL JVUI_ShowPopupMenu(HWND hWnd, POINT *curpos, int wDefaultItem)
{
	// Add Menu Items
	HMENU hPopMenu = CreatePopupMenu();

	InsertMenuW(hPopMenu, 0, MF_BYPOSITION | MF_STRING, ID_ABOUT, L"About");
	InsertMenuW(hPopMenu, 1, MF_BYPOSITION | MF_STRING, ID_HELP, L"Help");
	InsertMenuW(hPopMenu, 2, MF_BYPOSITION | MF_STRING, ID_LICENSE, L"License");
	InsertMenuW(hPopMenu, 5, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenuW(hPopMenu, 6, MF_BYPOSITION | MF_STRING | MF_DISABLED, ID_STATE_BANNER, L"Current State");
	InsertMenuW(hPopMenu, 7, MF_BYPOSITION | MF_STRING | MF_DISABLED, ID_STATE_INFO, g_state ? L"- Running" : L"- Not running");
	InsertMenuW(hPopMenu, 8, MF_BYPOSITION | MF_STRING, ID_TOGGLE, L"Toggle");
	InsertMenuW(hPopMenu, 10, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
	InsertMenuW(hPopMenu, 11, MF_BYPOSITION | MF_STRING, ID_EXIT, L"Exit");

	SetMenuDefaultItem(hPopMenu, ID_ABOUT, FALSE);
	SetFocus(hWnd);
	SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hPopMenu, 0);

	// Show Popup Menu
	POINT pt;
	if (!curpos)
	{
		GetCursorPos(&pt);
		curpos = &pt;
	}

	WORD cmd = TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos->x, curpos->y, 0,hWnd, NULL);
	SendMessage(hWnd, WM_COMMAND, cmd, 0);

	DestroyMenu(hPopMenu);

	return 0;
}

void JVUI_AddTrayIcon(HWND hWnd, UINT uID, UINT flag, UINT uCallbackMsg, LPCWSTR lpInfoStr)
{
	NOTIFYICONDATAW nid;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));

	// Notification Icon
	nid.cbSize 		= sizeof(NOTIFYICONDATAW);
	nid.hWnd 		= hWnd;
	nid.uID 		= uID;
	nid.uFlags 		= NIF_ICON | flag;
	nid.dwInfoFlags = NIIF_NOSOUND | NIIF_USER;

	 // Don't throw an message
	if (uCallbackMsg)
		nid.uCallbackMessage = uCallbackMsg;

#ifdef _MSC_VER
	LoadIconMetric(g_hInst, MAKEINTRESOURCEW(IDI_MAINICON), LIM_SMALL, &(nid.hIcon)); // Load the icon for high DPI. However, MinGW-w64 cannot link this function...
#else
	nid.hIcon 		= (HICON) LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_MAINICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
#endif
	StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), JV_SYSTRAY_TIP);
	StringCchCopyW(nid.szInfo, ARRAYSIZE(nid.szInfo), lpInfoStr);
	nid.uVersion 	= NOTIFYICON_VERSION;

	Shell_NotifyIcon(NIM_SETVERSION, &nid);
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void JVUI_DelTrayIcon(HWND hWnd, UINT uID)
{
	NOTIFYICONDATAW nid;
	ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
	nid.cbSize = sizeof(NOTIFYICONDATAW);
	nid.hWnd = hWnd;
	nid.uID = uID;

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void JVUI_WM_CLOSE(HWND hWnd, uint8_t postquit)
{
	JVUI_DelTrayIcon(hWnd, JV_SYSTRAY_ID_ON);
	JV_TurnOff(g_dllName);
	if (postquit)
		PostQuitMessage(WM_QUIT);
}


void JVUI_OpenLicense(HWND hWnd)
{
	// Open GitHub repository's LICENSE page
	ShellExecuteW(hWnd, L"open", L"https://github.com/ied206/Notepad-UTF8/blob/master/LICENSE", NULL, NULL, SW_SHOW);
}

void JVUI_PrintBanner(HWND hWnd)
{
	WCHAR msg[JV_BUF_SIZE];
	StringCchPrintfW(msg, JV_BUF_SIZE,
						L"Joveler's Notepad-UTF8 v%d.%d (%dbit)\n"
						L"Set Notepad's default encoding from ANSI to UTF-8.\n\n"
						L"[Binary] %s\n"
						L"[Source] %s\n\n"
						L"Build %04d%02d%02d",
						JV_VER_MAJOR, JV_VER_MINOR, JV_GetProcArch(),
						JV_WEB_BINARY, JV_WEB_SOURCE,
						CompileYear(), CompileMonth(), CompileDay());
	printf("%S\n\n", msg);
	MessageBoxW(hWnd, msg, L"Notepad-UTF8", MB_ICONINFORMATION | MB_OK);
}

void JVUI_PrintHelp(HWND hWnd)
{
	WCHAR* msg = 	L"[Notepad-UTF8 Help Message]\n"
					L"Set Notepad's default encoding from ANSI to UTF-8.\n\n"
					L"[Command Line Option]\n"
					L"-q : Launch this program without notification.\n"
					L"-h : Print this help message and exit.\n\n"
					L"[Toggle]\n"
					L"If Notepad-UTF8 is running, it will be applied to every new Notepad.\n"
					L"If Notepad-UTF8 is not running, Notepad will be untouched.";
	printf("%S\n\n", msg);
	MessageBoxW(hWnd, msg, L"Notepad-UTF8", MB_ICONINFORMATION | MB_OK);
}
