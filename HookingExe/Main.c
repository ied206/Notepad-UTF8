#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <windows.h>
#include <tlhelp32.h>
#include <strsafe.h>
// #include "MinHook.h"

BOOL SetPrivilege(LPCWSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	HANDLE hProcess, hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

	hProcess = GetCurrentProcess();
    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        fprintf(stderr, "OpenProcessToken error: %lu\n", GetLastError());
        exit(1);
    }

    if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid))
    {
        fprintf(stderr, "LookupPrivilegeValue error: %lu\n", GetLastError());
        exit(1);
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        tp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES) NULL, (PDWORD) NULL))
    {
        printf("AdjustTokenPrivileges error: %lu\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        printf("The token does not have the specified privilege. \n");
        return FALSE;
    }

    return TRUE;
}

DWORD FindProcessID(LPCWSTR procName);
void InjectCode(DWORD dwPID, LPVOID baseAddress);
void* FindBaseAddress(DWORD dwPID);

int main(int argc, char* argv[])
{
	WCHAR procName[MAX_PATH];
	DWORD dwPID = 0;
	void* baseAddress = NULL;

	wcsncpy(procName, L"notepad.exe", MAX_PATH);
	procName[MAX_PATH-1] = '\0';

	// Get Debug Privilege
	SetPrivilege(SE_DEBUG_NAME, TRUE);
	// Get PID
    dwPID = FindProcessID(procName);
    printf("%S's PID = %lu\n", procName, dwPID);

    baseAddress = FindBaseAddress(dwPID);
	InjectCode(dwPID, baseAddress);

	return 0;
}

DWORD FindProcessID(LPCWSTR procName)
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
		fprintf(stderr, "Cannot find first Process, Error Code : %lu\n", GetLastError());
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

void InjectCode(DWORD dwPID, LPVOID baseAddress)
{
	HANDLE hProcess;
	DWORD oldFlag = 0, newFlag = 0;
	void* _g_ftOpenAs = baseAddress + 0x22260;
	uint8_t srcByte[4] = {3, 0, 0, 0};

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
	if (!hProcess)
    {
        fprintf(stderr, "OpenProcess(%lu) failed!!!\n", dwPID);
		exit(1);
    }

	VirtualProtect((void*) _g_ftOpenAs, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldFlag);
	WriteProcessMemory(hProcess, _g_ftOpenAs, (LPVOID) srcByte, sizeof(DWORD), NULL);
	VirtualProtect((void*) _g_ftOpenAs, sizeof(DWORD), oldFlag, &newFlag);

	CloseHandle(hProcess);
}

/* Find Base Address of process */
void* FindBaseAddress(DWORD dwPID)
{
    HANDLE hModule = INVALID_HANDLE_VALUE;
    MODULEENTRY32W me;
    void* procBaseAddress = (void*) 0;

    /* Take snapshot of all the modules in the process */
    hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

    /* Snapshot failed */
    if (hModule == INVALID_HANDLE_VALUE)
    {
    	fprintf(stderr, "Module Snapshot error\n");
    	exit(1);
    }

    /* Size the structure before usage */
    me.dwSize = sizeof(MODULEENTRY32W);

    /* Retrieve information about the first module */
    if (!Module32First(hModule, &me))
    {
    	fprintf(stderr, "First module not found\n");
    	exit(1);
    }

	/* Find module of the executable */
	do
	{
		/* Compare the name of the process to the one we want */
		if (me.th32ProcessID == dwPID)
		{
			procBaseAddress = (void*) me.modBaseAddr;
			break;
		}
	}
	while (Module32Next(hModule, &me));

    /* Found module and base address successfully */
    printf("Base Address : 0x%"PRIx64"\n\n", (uint64_t) procBaseAddress);
    CloseHandle(hModule);

    return procBaseAddress;
}
