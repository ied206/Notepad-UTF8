#ifndef INJECT_H_INCLUDED
#define INJECT_H_INCLUDED

#include <windows.h>

#define JV_DEBUG_PRIV_SUCCESS	0
#define JV_DEBUG_PRIV_FAILURE	1
#define JV_DEBUG_PRIV_NO_ADMIN	2

/// Turn on/off dll injection
void JV_TurnOn(WCHAR* dllFullPath);
void JV_TurnOff(WCHAR* dllName);
/// Inject/Eject to one process - by PID
BOOL JV_InjectDllByPID(const DWORD dwPID, const WCHAR *szDllPath);
BOOL JV_EjectDllByPID(const DWORD dwPID, const void* baseAddr);
BYTE* JV_GetDllAddressFromPID(const DWORD dwPID, const WCHAR* dllName);
/// Inject/Eject to specific processses
BOOL JV_InjectByProcName(const WCHAR* procName, const WCHAR* dllFullPath);
BOOL JV_EjectByProcName(const WCHAR* procName, const WCHAR* dllName);
DWORD JV_FindProcessID(const WCHAR* procName);
/// Inject/Eject to all process - Globally
BOOL JV_GlobalInject(const WCHAR* dllFullPath);
BOOL JV_GlobalEject(const WCHAR* dllName);
/// Use SetWindowsHookEx Injection Method
BOOL JV_SetMessageHook(const WCHAR* dllFullPath);
/// Get Debug Privilege - needs admin privilege
DWORD JV_GetDebugPrivilege();


#endif // INJECT_H_INCLUDED
