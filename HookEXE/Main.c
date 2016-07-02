#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

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

bool JV_ParseArg(int argc, char* argv[], JV_ARG* arg);
void JV_Help();
WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize);
BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize);

int main(int argc, char* argv[])
{
	WCHAR dllFullPath[MAX_PATH];
	WCHAR dllName[MAX_PATH];
	DWORD procArch = JV_GetProcArch();
	DWORD hostArch = JV_GetHostArch();
	JV_ARG arg;

	// Print program banner
	printf(	"Joveler's NotepadUTF8 v%d.%d (Compile %4d.%02d.%02d)\n"
			"- Set notepad's default encoding to UTF-8 instead of ANSI\n"
			"- Source  (Web) : %s\n"
			"- Release (Web) : %s\n\n",
			JV_VER_MAJOR, JV_VER_MINOR,
			CompileYear(), CompileMonth(), CompileDate(),
			JV_WEB_SOURCE, JV_WEB_RELEASE);

	// Parse argument
	JV_ParseArg(argc, argv, &arg);
	if (arg.help)
	{
		JV_Help();
		return 0;
	}

	// Check bitness
	if (hostArch != procArch)
	{
        fprintf(stderr, "You must use %lubit NotepadUTF8 for %lubit Windows\n\n", hostArch, hostArch);
        exit(1);
	}

	// Get dll name (NotepadUTF8_x64.dll || NotepadUTF8_x86.dll)
	JV_GetDllName(dllName, sizeof(dllName));

	// Try to Get Debug Privilege
	/*
	switch (JV_GetDebugPrivilege())
	{
	case JV_DEBUG_PRIV_SUCCESS:
		puts("Running with administrator privilege\n");
		break;
	case JV_DEBUG_PRIV_FAILURE:
		puts("Unable to obtain debug privilege\n");
		break;
	case JV_DEBUG_PRIV_NO_ADMIN:
		puts("Running without administrator privilege\n");
		break;
	}
	*/

	// Get dll full path
	JV_GetDllFullPath(dllFullPath, sizeof(dllFullPath));

	// Check DLL's existance
    if (!PathFileExistsW(dllFullPath))
	{
		fprintf(stderr, "[ERR] Unable to find ");
		if (hostArch == 32)
			fprintf(stderr, "%S\n\n", DLL_NAME_32);
		else if (hostArch == 64)
			fprintf(stderr, "%S\n\n", DLL_NAME_64);
		exit(1);
	}
	printf("dll path : %S\n", dllFullPath);

	// Inject dll to notepand / explorer
	JV_InjectByProcName(L"notepad.exe", dllFullPath);
	JV_InjectByProcName(L"explorer.exe", dllFullPath);
	printf("NotepadUTF8 is running...\nPress Enter to stop ");
	getchar();
	JV_EjectByProcName(L"notepad.exe", dllName);
	JV_EjectByProcName(L"explorer.exe", dllName);

	return 0;
}

// -m api
// -m msg
bool JV_ParseArg(int argc, char* argv[], JV_ARG* arg)
{
	bool flag_err = false;

	memset(arg, 0, sizeof(JV_ARG));
	// set to default value
	arg->method = JV_ARG_METHOD_API;
	arg->help = JV_ARG_HELP_OFF;

	if (2 <= argc)
	{
		for (int i = 1; i < argc; i++)
		{
			flag_err = FALSE;
            if (stricmp(argv[i], "-m") == 0 || stricmp(argv[i], "/m") == 0)
			{
				if (!(i+1 < argc))
					flag_err = true;
				else
				{
                    if (stricmp(argv[i+1], "api") == 0)
						arg->method = JV_ARG_METHOD_API;
					else if (stricmp(argv[i+1], "msg") == 0)
						arg->method = JV_ARG_METHOD_MSG;
					else
						flag_err = true;
				}
			}

			if (stricmp(argv[i], "-h") == 0 || stricmp(argv[i], "/?") == 0)
				arg->help = JV_ARG_HELP_ON;
		}

		if (flag_err)
		{
			fprintf(stderr, "[ERR] Invalid argument\n\n");
			exit(1);
		}
	}

	// return Zero when success
	return flag_err;
}

void JV_Help()
{
    printf(	"NotepadUTF8 [-m api | msg] [-h]\n"
			"  -m    choose method\n"
			"  -h    print this help message\n");
    exit(0);
}

WCHAR* JV_GetDllFullPath(WCHAR* dllFullPath, const size_t bufSize)
{
	WCHAR dllName[MAX_PATH];
	JV_GetDllName(dllName, sizeof(dllName));

	GetCurrentDirectoryW(bufSize, dllFullPath);
	StringCbCatW(dllFullPath, bufSize, L"\\");
	StringCbCatW(dllFullPath, bufSize, dllName);

	return dllFullPath;
}

BOOL JV_GetDllName(WCHAR* dllName, const size_t bufSize)
{
	DWORD procArch = JV_GetProcArch();
	switch (procArch)
	{
	case 32:
		StringCbCopyW(dllName, bufSize, DLL_NAME_32);
		break;
	case 64:
		StringCbCopyW(dllName, bufSize, DLL_NAME_64);
		break;
	default:
		fprintf(stderr, "[ERR] Unknown Windows Architecture\n\n");
		return FALSE;
		break;
	}
	return TRUE;
}
