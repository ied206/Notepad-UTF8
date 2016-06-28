#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

#include <windows.h>

typedef BOOL (WINAPI *GETSAVEFILENAMEW)(LPOPENFILENAME);
// typedef UINT_PTR (CALLBACK WINAPI *OFNHOOKPROC)(HWND, UINT, WPARAM, LPARAM);;

UINT_PTR CALLBACK DetourSaveDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI DetourGetSaveFileNameW(LPOPENFILENAME lpofn);
void HookGetSaveFileName();

#endif // HOOK_H_INCLUDED
