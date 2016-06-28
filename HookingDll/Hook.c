#include <windows.h>
#include "minhook.h"
#include "Hook.h"

// Pointer for calling original GetSaveFileNameW.
GETSAVEFILENAMEW fpGetSaveFileNameW = NULL;
LPOFNHOOKPROC fpSaveDialogHookProc = NULL;

UINT_PTR CALLBACK DetourSaveDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	// 0x20 : ANSI, 0x21 : UTF-16 LE, 0x22 : UTF-16 BE, 0x23 : UTF-8
    return fpSaveDialogHookProc(hdlg, uiMsg, wParam, 0x23);
}

// Detour function which overrides GetSaveFileNameW.
BOOL WINAPI DetourGetSaveFileNameW(LPOPENFILENAME lpofn)
{
	fpSaveDialogHookProc = lpofn->lpfnHook;
	lpofn->lpfnHook = DetourSaveDialogHookProc;
    return fpGetSaveFileNameW(lpofn);
}

void HookGetSaveFileName()
{
	if (MH_Initialize() != MH_OK)
		exit(1);

	if (MH_CreateHookApiEx(L"comdlg32", "GetSaveFileNameW", &DetourGetSaveFileNameW, (LPVOID) &fpGetSaveFileNameW, NULL) != MH_OK)
		exit(1);

	if (MH_EnableHook(&GetSaveFileNameW) != MH_OK)
		exit(1);

	MessageBoxW(NULL, L"Not Hooked...", L"MinHook Sample", MB_OK);

	if (MH_DisableHook(&GetSaveFileNameW) != MH_OK)
		exit(1);

	if (MH_Uninitialize() != MH_OK)
		exit(1);
}
