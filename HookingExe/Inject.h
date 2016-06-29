#ifndef INJECT_H_INCLUDED
#define INJECT_H_INCLUDED

#include <windows.h>

BOOL JV_InjectDLL(DWORD dwPID, WCHAR *szDllPath);
BOOL JV_InjectNotepad(WCHAR* procName, WCHAR* dllFullPath);
DWORD JV_FindProcessID(WCHAR* procName);
BOOL JV_GetDebugPrivilege();
BOOL JV_SetHook(WCHAR* procName, WCHAR* dllFullPath);

#endif // INJECT_H_INCLUDED
