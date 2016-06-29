#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <windows.h>
// #include "MinHook.h"

#include "Inject.h"
#include "Host.h"

int main(int argc, char* argv[])
{
	WCHAR* procName = L"notepad.exe";
	DWORD dwPID = 0;
	BOOL result = FALSE;

	// Get Debug Privilege
	// return FALSE if failed
	JV_GetDebugPrivilege();

	// Get PID
    dwPID = FindProcessID(procName);
    printf("%S's PID = %lu\n", procName, dwPID);

    // Inject Dll
    result = InjectDLL(dwPID, L"D:\\Jang\\Build\\Source\\C\\NotepadUTF8\\HookingDll\\bin\\Release64\\HookingDll.dll");
    if (result)
		printf("Inject success\n");
	else
		printf("Inject failed\n");

	return 0;
}

