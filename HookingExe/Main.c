#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <shlwapi.h>
#include <windows.h>
#include <strsafe.h>

// #include "MinHook.h"

#include "Inject.h"
#include "Host.h"
#include "BasicIO.h"

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath);
BOOL JV_GetDllName(WCHAR** dllName);

int main(int argc, char* argv[])
{
	WCHAR dllFullPath[MAX_PATH];
	WCHAR* dllName = NULL;
	DWORD procArch = JV_GetProcArch();

	JV_GetDllName(&dllName);
	// Get Debug Privilege
	// return FALSE if failed
	// JV_GetDebugPrivilege();

	// Get dll Path
	JV_GetDllFullPath(dllFullPath);

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
	// JV_InjectNotepad(L"notepad.exe", dllFullPath);

	// Set Global Hooking
	// JV_InjectAll(dllFullPath);
	JV_EjectAll(dllName);

	return 0;
}

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath)
{
	WCHAR* dllName = NULL;
	JV_GetDllName(&dllName);

	GetCurrentDirectoryW(MAX_PATH, dllFullPath);
	StringCchCatW(dllFullPath, MAX_PATH, L"\\");
	StringCchCatW(dllFullPath, MAX_PATH, dllName);

	return dllFullPath;
}

BOOL JV_GetDllName(WCHAR** dllName)
{
	DWORD procArch = JV_GetProcArch();
	switch (procArch)
	{
	case 32:
		*dllName = DLL_NAME_32;
		break;
	case 64:
		*dllName = DLL_NAME_64;
		break;
	default:
		fprintf(stderr, "[ERR] Unknown Windows Architecture\n\n");
		return FALSE;
		break;
	}
	return TRUE;
}
