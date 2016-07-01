#include "Var.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <shlwapi.h>
#include <windows.h>
#include <tlhelp32.h>

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

typedef BOOL (*fp_MessageHookStart_t)();
typedef BOOL (*fp_MessageHookStop_t)();


/// Inject/Eject to one process - by PID
BOOL JV_InjectDllByPID(const DWORD dwPID, const WCHAR *szDllPath)
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

    dwBufSize = lstrlenW(szDllPath) * 2 + 1;
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

BOOL JV_EjectDllByPID(const DWORD dwPID, const void* baseAddr)
{
	HANDLE hProcess, hThread;
	JV_WIN_VER winVer;
	fp_NtCreateThreadEx_t fp_NtCreateThreadEx = NULL;
	LPTHREAD_START_ROUTINE remoteThreadProc = NULL;

	if (!baseAddr)
		return FALSE;

	JV_GetHostVer(&winVer);
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
    {
        fprintf(stderr, "[ERR] OpenProcess(%lu) failed\nError Code : %lu\n\n", dwPID, GetLastError());
		return FALSE;
    }

	remoteThreadProc = (LPTHREAD_START_ROUTINE) GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "FreeLibrary");
	if (JV_CompareWinVer(&winVer, JV_CMP_GE, JV_CMP_L2, 6, 0, 0, 0))
	{ // Vista <= WinVer
		fp_NtCreateThreadEx = (fp_NtCreateThreadEx_t) GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateThreadEx");
		fp_NtCreateThreadEx(&hThread, 0x2000000, NULL, hProcess, remoteThreadProc, (void*) baseAddr, FALSE, 0, NULL, NULL, NULL);
	}
	else
	{ // 2000, XP
		hThread = CreateRemoteThread(hProcess, NULL, 0, remoteThreadProc, (void*) baseAddr, 0, NULL);
	}

	WaitForSingleObject(hThread, INFINITE);
	if (!hThread)
		return FALSE;

	CloseHandle(hProcess);
	CloseHandle(hThread);

	return TRUE;
}

BYTE* JV_GetDllAddressFromPID(const DWORD dwPID, const WCHAR* dllName)
{
    HANDLE hModule = INVALID_HANDLE_VALUE;
    MODULEENTRY32W me;
    void* procBaseAddr = NULL;

    // Take snapshot of moudles in this process
    hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModule == INVALID_HANDLE_VALUE)
    {
    	fprintf(stderr, "[ERR] CreateToolhelp32Snapshot() failed\nError Code : %lu\n\n", GetLastError());
    	return 0;
    }

    // Set me.dwSize
    me.dwSize = sizeof(MODULEENTRY32W);

    // Get Info of first module
    if (!Module32First(hModule, &me))
    {
    	fprintf(stderr, "[ERR] Molule32First() failed\nError Code : %lu\n\n", GetLastError());
    	return 0;
    }

	// Iterate modules
	do
	{
		// Get base address
		if (StrCmpIW(me.szModule, dllName) == 0)
		{
			procBaseAddr = (void*) me.modBaseAddr;
			break;
		}
	}
	while (Module32Next(hModule, &me));
    // Close Snapshot
    CloseHandle(hModule);

    return (BYTE*) procBaseAddr;
}


/// Inject/Eject to specific processses
BOOL JV_InjectByProcName(const WCHAR* procName, const WCHAR* dllFullPath)
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
			result = JV_InjectDllByPID(pe.th32ProcessID, dllFullPath);
			if (result)
				printf("[%5lu] %S Inject Success\n", pe.th32ProcessID, pe.szExeFile);
			else
				printf("[%5lu] %S Inject Failed\n", pe.th32ProcessID, pe.szExeFile);
		}
	}
	while (Process32NextW(hSnapShot, &pe));
	CloseHandle(hSnapShot);

	return TRUE;
}

BOOL JV_EjectByProcName(const WCHAR* procName, const WCHAR* dllName)
{
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32W pe;
	BOOL result;
	void* dllBaseAddr = NULL;

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
			dllBaseAddr = JV_GetDllAddressFromPID(pe.th32ProcessID, dllName);
			if (dllBaseAddr == NULL)
			{
				printf("[%5lu] %S does not have %S\n", pe.th32ProcessID, pe.szExeFile, dllName);
			}
			else
			{
				result = JV_EjectDllByPID(pe.th32ProcessID, (void*) dllBaseAddr);
				if (result)
					printf("[%5lu] %S Eject Success\n", pe.th32ProcessID, pe.szExeFile);
				else
					printf("[%5lu] %S Eject Failed\n", pe.th32ProcessID, pe.szExeFile);
			}
		}
	}
	while (Process32NextW(hSnapShot, &pe));
	CloseHandle(hSnapShot);

	return TRUE;
}

DWORD JV_FindProcessID(const WCHAR* procName)
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
		if (StrCmpIW(procName, pe.szExeFile) == 0)
		{
			dwPID = pe.th32ProcessID;
			break;
		}
	}
	while (Process32NextW(hSnapShot, &pe));

	CloseHandle(hSnapShot);
	return dwPID;
}


/// Inject/Eject to all process - Globally
BOOL JV_GlobalInject(const WCHAR* dllFullPath)
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
		result = JV_InjectDllByPID(pe.th32ProcessID, dllFullPath);
		if (result)
			printf("[%5lu] %S Inject Success\n", pe.th32ProcessID, pe.szExeFile);
		else
			printf("[%5lu] %S Inject Failed\n", pe.th32ProcessID, pe.szExeFile);
	}
	while (Process32NextW(hSnapShot, &pe));
	CloseHandle(hSnapShot);

	return TRUE;
}

BOOL JV_GlobalEject(const WCHAR* dllName)
{
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32W pe;
	BOOL result;
	void* dllBaseAddr = NULL;

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
		dllBaseAddr = JV_GetDllAddressFromPID(pe.th32ProcessID, dllName);
		if (dllBaseAddr == NULL)
		{
			printf("[%5lu] %S does not have %S\n", pe.th32ProcessID, pe.szExeFile, dllName);
		}
		else
		{
			result = JV_EjectDllByPID(pe.th32ProcessID, (void*) dllBaseAddr);
			if (result)
				printf("[%5lu] %S Eject Success\n", pe.th32ProcessID, pe.szExeFile);
			else
				printf("[%5lu] %S Eject Failed\n", pe.th32ProcessID, pe.szExeFile);
		}
	}
	while (Process32NextW(hSnapShot, &pe));
	CloseHandle(hSnapShot);

	return TRUE;
}


/// Use SetWindowsHookEx Injection Method
/// TODO : System becomes too slow or unstable
BOOL JV_SetMessageHook(const WCHAR* dllFullPath)
{
	HMODULE hDll = INVALID_HANDLE_VALUE;
	fp_MessageHookStart_t JV_MessageHookStart = NULL;
	fp_MessageHookStop_t JV_MessageHookStop = NULL;

	hDll = LoadLibraryExW(dllFullPath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hDll)
	{
		fprintf(stderr, "[ERR] LoadLibraryEx() failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}

	JV_MessageHookStart = (fp_MessageHookStart_t) GetProcAddress(hDll, "JV_MessageHookStart");
	JV_MessageHookStop = (fp_MessageHookStop_t) GetProcAddress(hDll, "JV_MessageHookStop");
	if (!JV_MessageHookStart || !JV_MessageHookStop)
	{
		fprintf(stderr, "[ERR] GetProcAddress() failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}

	JV_MessageHookStart();
	printf("Hooking Windows Messages...\nPress Enter to Exit...\n");
	getchar();
	JV_MessageHookStop();
	printf("Stopped hooking Windows Messages\n");

	FreeLibrary(hDll);

	return TRUE;
}


/// Get Debug Privilege - needs admin privilege
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
