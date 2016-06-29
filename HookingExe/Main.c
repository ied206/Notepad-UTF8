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

BOOL JV_GetDllFullPath(WCHAR* dllFullPath);

int main(int argc, char* argv[])
{
	WCHAR dllFullPath[MAX_PATH];
	WCHAR* procName = L"notepad.exe";
	DWORD procArch = JV_GetProcArch();


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
	// JV_InjectNotepad(procName, g_dllFullPath);

	// Set Message Hook
	JV_SetHook(procName, dllFullPath);


	return 0;
}

BOOL JV_GetDllFullPath(WCHAR* dllFullPath)
{
	DWORD procArch = 0;

	GetCurrentDirectoryW(MAX_PATH, dllFullPath);
	procArch = JV_GetProcArch();
	switch (procArch)
	{
	case 32:
		StringCchCatW(dllFullPath, MAX_PATH, L"\\");
		StringCchCatW(dllFullPath, MAX_PATH, DLL_NAME_32);
		break;
	case 64:
		StringCchCatW(dllFullPath, MAX_PATH, L"\\");
		StringCchCatW(dllFullPath, MAX_PATH, DLL_NAME_64);
		break;
	default:
		fprintf(stderr, "[ERR] Unknown Windows Architecture\n\n");
		return FALSE;
		break;
	}
	return TRUE;
}
