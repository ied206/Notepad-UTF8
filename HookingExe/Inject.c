#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <shlwapi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <strsafe.h>

#include "Inject.h"
#include "Host.h"


typedef DWORD (WINAPI *fp_NtCreateThreadEx_t)(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    LPVOID ObjectAttributes,
    HANDLE ProcessHandle,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    BOOL CreateSuspended,
    DWORD dwStackSize,
    LPVOID Unknown1,
    LPVOID Unknown2,
    LPVOID Unknown3);

typedef BOOL (*fp_HookStart_t)();
typedef BOOL (*fp_HookStop_t)();

BOOL JV_InjectDLL(DWORD dwPID, WCHAR *szDllPath)
{
	HANDLE hProcess, hThread;
	JV_WIN_VER winVer;
	fp_NtCreateThreadEx_t fp_NtCreateThreadEx = NULL;
	LPTHREAD_START_ROUTINE remoteThreadProc = NULL;
	DWORD dwBufSize = 0;
	void* remoteProcBuf = NULL;

	JV_GetHostVer(&winVer);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
    {
        fprintf(stderr, "[ERR] OpenProcess(%lu) failed\nError Code : %lu\n\n", dwPID, GetLastError());
		return FALSE;
    }

    dwBufSize = wcslen(szDllPath) * 2 + 1;
    remoteProcBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteProcBuf)
		return FALSE;
    WriteProcessMemory(hProcess, remoteProcBuf, (void*)szDllPath, dwBufSize, NULL);
	remoteThreadProc = (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");

	if (JV_CompareWinVer(&winVer, JV_CMP_GE, JV_CMP_L2, 6, 0, 0, 0))
	{ // Vista <= WinVer
		fp_NtCreateThreadEx = (fp_NtCreateThreadEx_t) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateThreadEx");
		fp_NtCreateThreadEx(&hThread, 0x2000000, NULL, hProcess, remoteThreadProc, remoteProcBuf, FALSE, 0, NULL, NULL, NULL);
	}
	else
	{ // 2000, XP
		hThread = CreateRemoteThread(hProcess, NULL, 0, remoteThreadProc, remoteProcBuf, 0, NULL);
	}

	WaitForSingleObject(hThread, INFINITE);
	VirtualFreeEx(hProcess, remoteProcBuf, 0, MEM_RELEASE);
	if (!hThread)
		return FALSE;

	CloseHandle(hProcess);
	CloseHandle(hThread);

	return TRUE;
}

//	DWORD oldFlag = 0, newFlag = 0;
//	VirtualProtect((void*) _g_ftOpenAs, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldFlag);
// 	WriteProcessMemory(hProcess, _g_ftOpenAs, (LPVOID) srcByte, sizeof(DWORD), NULL);
//	VirtualProtect((void*) _g_ftOpenAs, sizeof(DWORD), oldFlag, &newFlag);

BOOL JV_InjectNotepad(WCHAR* procName, WCHAR* dllFullPath)
{
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32W pe;
	BOOL result;

	// Get the snapshot of the system
	pe.dwSize = sizeof(PROCESSENTRY32W);
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// find process
	if (!Process32FirstW(hSnapShot, &pe))
	{
		fprintf(stderr, "[ERR] Process32FirstW() failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}

	do
	{
		if (StrCmpIW(procName, pe.szExeFile) == 0)
		{
			result = JV_InjectDLL(pe.th32ProcessID, dllFullPath);
			if (result)
				printf("[%5lu] Inject Success\n", pe.th32ProcessID);
			else
				printf("[%5lu] Inject Failed\n", pe.th32ProcessID);
		}
	}
	while (Process32NextW(hSnapShot, &pe));
	CloseHandle(hSnapShot);

	return TRUE;
}


DWORD JV_FindProcessID(WCHAR* procName)
{
	DWORD dwPID = 0;
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32W pe;

	// Get the snapshot of the system
	pe.dwSize = sizeof(PROCESSENTRY32W);
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// find process
	if (!Process32FirstW(hSnapShot, &pe))
	{
		fprintf(stderr, "[ERR] Process32FirstW() failed\nError Code : %lu\n\n", GetLastError());
		exit(1);
	}

	do
	{
		if (wcsicmp(procName, pe.szExeFile) == 0)
		{
			dwPID = pe.th32ProcessID;
			break;
		}
	}
	while (Process32NextW(hSnapShot, &pe));

	CloseHandle(hSnapShot);
	return dwPID;
}


BOOL JV_GetDebugPrivilege()
{
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hToken;
    TOKEN_PRIVILEGES pToken;
    LUID luid;

    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        fprintf(stderr, "[ERR] OpenProcessToken() failed\nError Code : %lu\n\n", GetLastError());
        return FALSE;
    }

    if (!LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &luid))
    {
        fprintf(stderr, "[ERR] LookupPrivilegeValue() failed\nError Code : %lu\n\n", GetLastError());
        return FALSE;
    }

    pToken.PrivilegeCount = 1;
    pToken.Privileges[0].Luid = luid;
    pToken.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &pToken, 0, (TOKEN_PRIVILEGES*) NULL, (DWORD*) NULL))
    {
        fprintf(stderr, "[ERR] AdjustTokenPrivileges() failed\nError Code : %lu\n\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        fprintf(stderr, "[ERR] ERROR_NOT_ALL_ASSIGNED\nTry running this program with Administrator Privilege.\n\n");
        return FALSE;
    }
    CloseHandle(hToken);

    return TRUE;
}

BOOL JV_SetHook(WCHAR* procName, WCHAR* dllFullPath)
{
	HMODULE hDll = INVALID_HANDLE_VALUE;
	fp_HookStart_t JV_HookStart = NULL;
	fp_HookStop_t JV_HookStop = NULL;

	hDll = LoadLibraryW(dllFullPath);
	if (!hDll)
	{
		fprintf(stderr, "[ERR] LoadLibrary() failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}

	JV_HookStart = (fp_HookStart_t) GetProcAddress(hDll, "JV_HookStart");
	JV_HookStop = (fp_HookStop_t) GetProcAddress(hDll, "JV_HookStop");
	if (!JV_HookStart || !JV_HookStop)
	{
		fprintf(stderr, "[ERR] GetProcAddress() failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}

	JV_HookStart();
	printf("Hooking Notepad.exe\nPress Enter to Exit...\n");
	getchar();
	JV_HookStop();

	FreeLibrary(hDll);

	return TRUE;
}
