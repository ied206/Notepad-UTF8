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


WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize);
BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize);
LRESULT CALLBACK WndProcedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE g_hInst;
HWND g_hWnd;
int g_state = JV_STATE_TURN_ON;
WCHAR g_dllFullPath[MAX_PATH];
WCHAR g_dllName[MAX_PATH];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	MSG	Msg;

	DWORD procArch = JV_GetProcArch();
	DWORD hostArch = JV_GetHostArch();

	// Init g_hInst
	g_hInst = hInstance;

	// Find if BatteryLine is already running.
	hWnd = FindWindowW(JV_CLASS_NAME, 0);
	if (hWnd != NULL) // Running BatteryLine found? Terminate it.
	{
		JVUI_AddTrayIcon(hWnd, JV_SYSTRAY_ID_OFF, NIF_INFO, 0, L"Notepad-UTF8 Off");
		JVUI_DelTrayIcon(hWnd, JV_SYSTRAY_ID_OFF);
		SendMessageW(hWnd, WM_CLOSE, 0, 0);
		return 0;
	}

	// Get dll name and full path (NotepadUTF8_x64.dll || NotepadUTF8_x86.dll)
	JV_GetDllName(g_dllName, sizeof(g_dllName));
	JV_GetDllFullPath(g_dllFullPath, sizeof(g_dllFullPath));
	// Check DLL's existance
    if (!PathFileExistsW(g_dllFullPath))
	{
		fprintf(stderr, "[ERR] Unable to find ");
		if (hostArch == 32)
			fprintf(stderr, "%S\n\n", DLL_NAME_32);
		else if (hostArch == 64)
			fprintf(stderr, "%S\n\n", DLL_NAME_64);
		exit(1);
	}
	printf("dll path : %S\n", g_dllFullPath);

	// Init Window
	g_hWnd = hWnd = JVUI_InitWindow(hInstance);

	// Check bitness
	if (hostArch != procArch)
	{
		WCHAR msgbox[JV_BUF_SIZE];
		StringCchPrintfW(msgbox, JV_BUF_SIZE, L"You must use %lubit Notepad-UTF8 for %lubit Windows\n\n", hostArch, hostArch);
        MessageBoxW(hWnd, msgbox, L"Error", MB_OK | MB_ICONERROR);
        exit(1);
	}


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

	return Msg.wParam;
}

LRESULT CALLBACK WndProcedure(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	WCHAR msgbox[JV_BUF_SIZE];

    switch (Msg)
    {
	case WM_CREATE:
		#ifdef _DEBUG_CONSOLE
		puts("WM_CREATE");
		#endif // _DEBUG_CONSOLE
		JVUI_AddTrayIcon(hWnd, JV_SYSTRAY_ID_ON, NIF_MESSAGE | NIF_TIP | NIF_INFO, WM_APP_SYSTRAY_POPUP, L"Notepad-UTF8 On");
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
				StringCchPrintfW(msgbox, JV_BUF_SIZE,
						L"Joveler's Notepad-UTF8 %d.%d (%dbit)\n"
						L"[Binary] %s\n"
						L"[Source] %s\n\n"
						L"Compile Date : %04d.%02d.%02d\n",
						JV_VER_MAJOR, JV_VER_MINOR, JV_GetProcArch(),
						JV_WEB_RELEASE, JV_WEB_SOURCE,
						CompileYear(), CompileMonth(), CompileDate());
				MessageBoxW(hWnd, msgbox, L"Notepad-UTF8", MB_ICONINFORMATION | MB_OK);
				break;
			case ID_TOGGLE:
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

/*
int main_bak(int argc, char* argv[])
{
	WCHAR dllFullPath[MAX_PATH];
	WCHAR dllName[MAX_PATH];
	DWORD procArch = JV_GetProcArch();
	DWORD hostArch = JV_GetHostArch();
	JV_ARG arg;

	// Print program banner
	printf(	"Joveler's NotepadUTF8 v%d.%d (Compile %4d.%02d.%02d)\n"
			"- Set notepad's default encoding to UTF-8 instead of ANSI\n"
			"- Source  (Web) : %s\n"
			"- Release (Web) : %s\n\n",
			JV_VER_MAJOR, JV_VER_MINOR,
			CompileYear(), CompileMonth(), CompileDate(),
			JV_WEB_SOURCE, JV_WEB_RELEASE);

	// Parse argument
	JV_ParseArg(argc, argv, &arg);
	if (arg.help)
	{
		JV_Help();
		return 0;
	}

	// Check bitness
	if (hostArch != procArch)
	{
        fprintf(stderr, "You must use %lubit NotepadUTF8 for %lubit Windows\n\n", hostArch, hostArch);
        exit(1);
	}

	// Get dll name (NotepadUTF8_x64.dll || NotepadUTF8_x86.dll)
	JV_GetDllName(dllName, sizeof(dllName));

	// Get dll full path
	JV_GetDllFullPath(dllFullPath, sizeof(dllFullPath));

	// Check DLL's existance
    if (!PathFileExistsW(dllFullPath))
	{
		fprintf(stderr, "[ERR] Unable to find ");
		if (hostArch == 32)
			fprintf(stderr, "%S\n\n", DLL_NAME_32);
		else if (hostArch == 64)
			fprintf(stderr, "%S\n\n", DLL_NAME_64);
		exit(1);
	}
	printf("dll path : %S\n", dllFullPath);

	// Inject dll to notepand / explorer
	JV_InjectByProcName(L"notepad.exe", dllFullPath);
	JV_InjectByProcName(L"explorer.exe", dllFullPath);
	printf("NotepadUTF8 is running...\nPress Enter to stop ");
	getchar();
	JV_EjectByProcName(L"notepad.exe", dllName);
	JV_EjectByProcName(L"explorer.exe", dllName);

	return 0;
}
*/

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize)
{
	WCHAR dllName[MAX_PATH];
	JV_GetDllName(dllName, sizeof(dllName));

	GetCurrentDirectoryW(bufSize, dllFullPath);
	StringCbCatW(dllFullPath, bufSize, L"\\");
	StringCbCatW(dllFullPath, bufSize, dllName);

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
