#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

#include <shlwapi.h>
#include <windows.h>
#ifdef _DEBUG
#undef __CRT__NO_INLINE
#endif
#include <strsafe.h>
#ifdef _DEBUG
#define __CRT__NO_INLINE
#endif

#include "Inject.h"
#include "Host.h"
#include "UI.h"
#include "BasicIO.h"

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
bool JV_CheckWindowVer();
bool JV_ParseArg(int argc, LPWSTR* argv, JV_ARG* arg);
WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize);
BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize);

HINSTANCE g_hInst;
HWND g_hWnd = NULL;
int g_state = JV_STATE_TURN_OFF;
WCHAR g_dllFullPath[MAX_PATH];
WCHAR g_dllName[MAX_PATH];
JV_ARG g_arg;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG	Msg;
	int argc = 0;
	LPWSTR* argv;
	DWORD procArch = JV_GetProcArch();
	DWORD hostArch = JV_GetHostArch();

	// Init g_hInst
	g_hInst = hInstance;

	// Get Command line arguments
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (JV_ParseArg(argc, argv, &g_arg))
	{
		fprintf(stderr, "[ERR] Invalid argument\n\n");
		MessageBox(g_hWnd, L"[ERR] Invalid argument", L"Error", MB_ICONERROR | MB_OK);
		exit(1);
	}

	// Print Help Message and exit if -h /? is used
	if (g_arg.help)
	{
        JVUI_PrintHelp();
        exit(0);
	}

	// Check if this windows is supported by Notepad-UTF8
	if (!JV_CheckWindowVer())
	{
		WCHAR msgbox[JV_BUF_SIZE];
		StringCchPrintfW(msgbox, JV_BUF_SIZE, L"[ERR] Notepad-UTF8 v%d.%d does not support this version of Windows.\n\nPlease check for an update by visiting project website :\n- %s", JV_VER_MAJOR, JV_VER_MINOR, JV_WEB_RELEASE);
		fprintf(stderr, "%S\n\n", msgbox);
        MessageBoxW(NULL, msgbox, L"Error", MB_OK | MB_ICONERROR);
        return 1;
	}

	// Check if Notepad-UTF8 is already running.
	hWnd = FindWindowW(JV_CLASS_NAME, 0);
	if (hWnd != NULL) // Running Notepad-UTF8 found? Terminate it.
	{
		JVUI_AddTrayIcon(hWnd, JV_SYSTRAY_ID_OFF, (g_arg.quiet ? 0x0 : NIF_INFO), 0, L"Notepad-UTF8 Off");
		JVUI_DelTrayIcon(hWnd, JV_SYSTRAY_ID_OFF);
		SendMessageW(hWnd, WM_CLOSE, 0, 0);
		return 0;
	}

	// Check bitness
	if (hostArch != procArch)
	{
		WCHAR msgbox[JV_BUF_SIZE];
		StringCchPrintfW(msgbox, JV_BUF_SIZE, L"[ERR] You must use Notepad-UTF8_x%lu.exe for %lubit Windows.", hostArch == 64 ? 64 : 86, hostArch);
		fprintf(stderr, "%S\n\n", msgbox);
        MessageBoxW(NULL, msgbox, L"Error", MB_OK | MB_ICONERROR);
        return 1;
	}

	// Get dll name and full path (NotepadUTF8_x64.dll || NotepadUTF8_x86.dll)
	JV_GetDllName(g_dllName, sizeof(g_dllName));
	JV_GetDllFullPath(g_dllFullPath, sizeof(g_dllFullPath));
	// Check DLL's existance
	#ifdef _DEBUG_CONSOLE
    printf("DLL Path : %S\n", g_dllFullPath);
	#endif // _DEBUG_CONSOLE
    if (!PathFileExistsW(g_dllFullPath))
	{
		WCHAR msgbox[JV_BUF_SIZE];
		StringCchCopyW(msgbox, JV_BUF_SIZE, L"[ERR] Unable to find ");
		if (hostArch == 32)
			StringCchCatW(msgbox, JV_BUF_SIZE, DLL_NAME_32);
		else if (hostArch == 64)
			StringCchCatW(msgbox, JV_BUF_SIZE, DLL_NAME_64);
		fprintf(stderr, "%S\n\n", msgbox);
        MessageBoxW(NULL, msgbox, L"Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	// Init Window
	g_hWnd = hWnd = JVUI_InitWindow(hInstance);

	// Do Dll Injection
	JV_TurnOn(g_dllFullPath);

	// Decode and treat the messages as long as the application is running
	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	// Destroy Window
	JVUI_WM_CLOSE(hWnd, FALSE);

	// return Msg.wParam;
	return 0;
}

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
    {
	case WM_CREATE:
		#ifdef _DEBUG_CONSOLE
		puts("WM_CREATE");
		#endif // _DEBUG_CONSOLE
		JVUI_AddTrayIcon(hWnd, JV_SYSTRAY_ID_ON, NIF_MESSAGE | NIF_TIP | (g_arg.quiet ? 0x0 : NIF_INFO), WM_APP_SYSTRAY_POPUP, L"Notepad-UTF8 On");
		break;
	case WM_APP_SYSTRAY_POPUP: // systray msg callback
		#ifdef _DEBUG_CONSOLE
		puts("WM_APP_SYSTRAY_POPUP");
		#endif
        switch (lParam)
        {
		case WM_LBUTTONDBLCLK:
			#ifdef _DEBUG_CONSOLE
			puts("  WM_LBUTTONDBLCLK");
			#endif // _DEBUG_CONSOLE
			SendMessage(hWnd, WM_COMMAND, ID_ABOUT, 0);
			break;
		case WM_RBUTTONUP:
			#ifdef _DEBUG_CONSOLE
			puts("  WM_RBUTTONUP");
			#endif // _DEBUG_CONSOLE
			SetForegroundWindow(hWnd);
			JVUI_ShowPopupMenu(hWnd, NULL, -1);
			PostMessage(hWnd, WM_APP_SYSTRAY_POPUP, 0, 0);
			break;
        }
        break;
	case WM_COMMAND: // systray msg callback
		#ifdef _DEBUG_CONSOLE
		puts("WM_COMMAND");
		#endif // _DEBUG_CONSOLE
        switch (LOWORD(wParam))
        {
            case ID_ABOUT:
				#ifdef _DEBUG_CONSOLE
				puts("  ID_ABOUT");
				#endif // _DEBUG_CONSOLE
				// Print program banner
				JVUI_PrintBanner();
				break;
			case ID_HELP:
				#ifdef _DEBUG_CONSOLE
				puts("  ID_HELP");
				#endif // _DEBUG_CONSOLE
				// Print help message
				JVUI_PrintHelp();
				break;
			case ID_LICENSE:
				JVUI_ViewLicense(g_hWnd);
				break;
			case ID_TOGGLE:
				#ifdef _DEBUG_CONSOLE
				puts("  ID_TOGGLE");
				#endif // _DEBUG_CONSOLE
                switch (g_state)
                {
				case JV_STATE_TURN_ON:
					JV_TurnOff(g_dllName);
					break;
				case JV_STATE_TURN_OFF:
					JV_TurnOn(g_dllFullPath);
					break;
                }
				break;
			case ID_EXIT:
				#ifdef _DEBUG_CONSOLE
				puts("  ID_EXIT");
				#endif // _DEBUG_CONSOLE
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
        }
        break;
	case WM_CLOSE: // 0x0010
		#ifdef _DEBUG_CONSOLE
		if (Msg == WM_CLOSE)
			puts("WM_CLOSE");
		else if (Msg == WM_DESTROY)
			puts("WM_DESTROY");
		#endif // _DEBUG_CONSOLE
		JVUI_WM_CLOSE(hWnd, TRUE);
        break;
	default:
		return DefWindowProc(hWnd, Msg, wParam, lParam);
		break;
    }

    return 0;
}

bool JV_ParseArg(int argc, LPWSTR* argv, JV_ARG* arg)
{
	bool flag_err = false;

	memset(arg, 0, sizeof(JV_ARG));
	// set to default value
	arg->quiet = JV_ARG_QUIET_OFF;
	arg->help = JV_ARG_HELP_OFF;

	if (2 <= argc)
	{
		for (int i = 1; i < argc; i++)
		{
			flag_err = false;
            if (StrCmpIW(argv[i], L"-q") == 0 || StrCmpIW(argv[i], L"--quiet") == 0 || StrCmpIW(argv[i], L"/q") == 0)
				arg->quiet = JV_ARG_QUIET_ON;
			else if (StrCmpIW(argv[i], L"-h") == 0 || StrCmpIW(argv[i], L"--help") == 0 || StrCmpIW(argv[i], L"/?") == 0 || StrCmpIW(argv[i], L"-?") == 0)
				arg->help = JV_ARG_HELP_ON;
		}
	}

	// return Zero when success
	return flag_err;
}

bool JV_CheckWindowVer()
{
    JV_WIN_VER wv;
    JV_GetHostVer(&wv);

	// XP SP3
	if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 5, 1, 2600, 5512))
		return true;
	// Vista SP2
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 6, 0, 6001, 18000))
		return true;
	// 7 (6.1.7600.16385, 6.1.7601.18917) (Tested)
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L2, 6, 1, 7600, 16385))
		return true;
	// 8 SP2
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 6, 2, 9200, 16384))
		return true;
	// 8.1 Update 1
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 6, 3, 9600, 17415))
		return true;
	// 10.0.10240
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 10, 0, 10240, 16384))
		return true;
	// 10.0.10586
	else if (JV_CompareWinVer(&wv, JV_CMP_E, JV_CMP_L3, 10, 0, 10586, 0))
		return true;
	else
		return false;
}

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize)
{
	WCHAR* dirPath = NULL;
	WCHAR dllName[MAX_PATH];
	JV_GetDllName(dllName, sizeof(dllName));

	//GetCurrentDirectoryW(bufSize, dllFullPath);
	//StringCbCatW(dllFullPath, bufSize, L"\\");
	//StringCbCatW(dllFullPath, bufSize, dllName);
	GetModuleFileNameW(NULL, dllFullPath, bufSize);
    dirPath = StrRChrW(dllFullPath, NULL, L'\\')+1;
	StringCbCopyW(dirPath, bufSize, dllName);

	return dllFullPath;
}

BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize)
{
	DWORD procArch = JV_GetProcArch();
	switch (procArch)
	{
	case 32:
		StringCbCopyW(dllName, bufSize, DLL_NAME_32);
		break;
	case 64:
		StringCbCopyW(dllName, bufSize, DLL_NAME_64);
		break;
	default:
		fprintf(stderr, "[ERR] Unknown Windows Architecture\n\n");
		return FALSE;
		break;
	}
	return TRUE;
}
