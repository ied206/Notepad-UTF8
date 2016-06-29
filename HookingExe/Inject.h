#ifndef INJECT_H_INCLUDED
#define INJECT_H_INCLUDED

#include <windows.h>

BOOL JV_InjectDLL(DWORD dwPID, WCHAR *szDllPath);
BOOL JV_EjectDLL(DWORD dwPID, void* baseAddr);
BOOL JV_InjectNotepad(WCHAR* procName, WCHAR* dllFullPath);
BOOL JV_InjectAll(WCHAR* dllFullPath);
BOOL JV_EjectAll(WCHAR* dllName);
BYTE* JV_GetBaseAddress_of_Dll(DWORD dwPID, WCHAR* szDllPath);
DWORD JV_FindProcessID(WCHAR* procName);
BOOL JV_GetDebugPrivilege();
BOOL JV_SetHook(WCHAR* dllFullPath);

#endif // INJECT_H_INCLUDED
