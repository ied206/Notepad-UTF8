#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

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
#include "BasicIO.h"

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize);
BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize);

int main(int argc, char* argv[])
{
	WCHAR dllFullPath[MAX_PATH];
	WCHAR dllName[MAX_PATH];
	DWORD procArch = JV_GetProcArch();

	JV_GetDllName(dllName, sizeof(dllName));
	// Get Debug Privilege
	JV_GetDebugPrivilege(); // If this return FALSE, it does not have admin privileges

	// Get dll Path
	JV_GetDllFullPath(dllFullPath, sizeof(dllFullPath));

	// Check DLL's existance
    if (!PathFileExistsW(dllFullPath))
	{
		fprintf(stderr, "[ERR] Unable to find ");
		if (procArch == 32)
			fprintf(stderr, "%S\n\n", DLL_NAME_32);
		else if (procArch == 64)
			fprintf(stderr, "%S\n\n", DLL_NAME_64);
		exit(1);
	}
	printf("dll path : %S\n", dllFullPath);

	// Hook running notepad
	// JV_InjectProcess(L"notepad.exe", dllFullPath);

	// Set Global Message Hook
	//JV_SetMessageHook(dllFullPath);

	// Set Global Notepad/CreateProcess Hook
	JV_GlobalInject(dllFullPath);
	printf("Hooking all processes...\nPress Enter to Exit...\n");
	getchar();
	printf("Stopped hooking all processes\n");
	JV_GlobalEject(dllName);

	return 0;
}

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize)
{
	WCHAR dllName[MAX_PATH];
	JV_GetDllName(dllName, sizeof(dllName));

	GetCurrentDirectoryW(bufSize, dllFullPath);
	StringCchCatW(dllFullPath, bufSize, L"\\");
	StringCchCatW(dllFullPath, bufSize, dllName);

	return dllFullPath;
}

BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize)
{
	DWORD procArch = JV_GetProcArch();
	switch (procArch)
	{
	case 32:
		StringCchCopyW(dllName, bufSize, DLL_NAME_32);
		break;
	case 64:
		StringCchCopyW(dllName, bufSize, DLL_NAME_64);
		break;
	default:
		fprintf(stderr, "[ERR] Unknown Windows Architecture\n\n");
		return FALSE;
		break;
	}
	return TRUE;
}
