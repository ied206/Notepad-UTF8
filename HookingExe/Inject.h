#ifndef INJECT_H_INCLUDED
#define INJECT_H_INCLUDED

#include <windows.h>

BOOL InjectDLL(DWORD dwPID, WCHAR *szDllPath);
DWORD FindProcessID(WCHAR* procName);
BOOL JV_GetDebugPrivilege();

#endif // INJECT_H_INCLUDED
