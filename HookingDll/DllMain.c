#include "Var.h"

#include <stdint.h>

#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#ifdef _DEBUG
#undef __CRT__NO_INLINE
#endif
#include <strsafe.h>
#ifdef _DEBUG
#define __CRT__NO_INLINE
#endif

#include "DllMain.h"
#include "MinHook.h"

/// Create Thread for dll Attach/Detach
DWORD WINAPI JV_DllProcessAttach(LPVOID lpParam);
DWORD WINAPI JV_DllProcessDetach(LPVOID lpParam);
/// Set Notepad's default encoding to UTF-8 (notepad.exe)
BOOL JV_SetNotepadUTF8();
BOOL JV_IsThisProcessNotepad();
BYTE* JV_GetBaseAddress(const DWORD dwPID);
BYTE* JV_GetNotepadOpenAsAddr(BYTE* baseAddr, const JV_WIN_VER* winVer, const DWORD hostArch);
/// Hook CreateProcessA, CreateProcessW (Non-Notepad)
BOOL JV_HookCreateProcess();
BOOL JV_UnHookCreateProcess();
BOOL JV_InjectDllByHandle(const HANDLE hProcess, const WCHAR *szDllPath);
/// Get Host Windows' Version/Architecture Information
BOOL JV_GetHostVer(JV_WIN_VER* winVer);
DWORD JV_GetHostArch();
DWORD JV_GetProcArch();
BOOL JV_CompareWinVer(const JV_WIN_VER* wv, const DWORD op, const DWORD effective, const WORD major, const WORD minor, const WORD bMajor, const WORD bMinor);
/// CreateProcessA, CreateProcessW Hooking for Non-Notepad
CREATEPROCESSA fpCreateProcessA = NULL;
CREATEPROCESSW fpCreateProcessW = NULL;
BOOL WINAPI MyCreateProcessA(
	LPCSTR                lpApplicationName,
	LPSTR                 lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCSTR                lpCurrentDirectory,
	LPSTARTUPINFO         lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation);
BOOL WINAPI MyCreateProcessW(
	LPCWSTR                lpApplicationName,
	LPWSTR                 lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR                lpCurrentDirectory,
	LPSTARTUPINFO         lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation);
/// Use SetWindowsHookEx Injection Method
LRESULT CALLBACK JV_CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
HHOOK g_hProcHook = NULL;
HINSTANCE g_hInstance = NULL;
BOOL g_isNotepad = FALSE;

// https://msdn.microsoft.com/en-us/library/windows/desktop/dn633971(v=vs.85).aspx
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms682596(v=vs.85).aspx
extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
    HANDLE hThread = NULL;
    // return FALSE to fail DLL load in DLL_PROCESS_ATTACH

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hInstDll;
            hThread = CreateThread(NULL, 0, JV_DllProcessAttach, NULL, 0, NULL);
            CloseHandle(hThread);
            break;
        case DLL_PROCESS_DETACH:
            // detach from process
            hThread = CreateThread(NULL, 0, JV_DllProcessDetach, NULL, 0, NULL);
            WaitForSingleObject(hThread, INFINITE);
            CloseHandle(hThread);
            break;
        case DLL_THREAD_ATTACH:
            // attach to thread
            break;
        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

/// Create Thread for dll Attach/Detach
DWORD WINAPI JV_DllProcessAttach(LPVOID lpParam)
{
    g_isNotepad = JV_IsThisProcessNotepad();
    if (g_isNotepad)
    {
        if (!JV_SetNotepadUTF8())
            return 1;
    }
    else
    {
        if (!JV_HookCreateProcess())
            return 1;
    }

    // 0 means Succesful
    return 0;
}

DWORD WINAPI JV_DllProcessDetach(LPVOID lpParam)
{
    if (g_isNotepad)
        JV_UnHookCreateProcess();

    // 0 means Succesful
    return 0;
}

/// Set Notepad's default encoding to UTF-8 (notepad.exe)
BOOL JV_SetNotepadUTF8()
{
    BYTE* procBaseAddr = NULL;
    DWORD* _g_ftOpenAs = NULL;
    JV_WIN_VER winVer;
    DWORD hostArch;

    procBaseAddr = JV_GetBaseAddress(GetProcessId(GetCurrentProcess()));
    if (!JV_GetHostVer(&winVer))
        return FALSE;
    hostArch = JV_GetHostArch();
    if (!hostArch)
        return FALSE;

    _g_ftOpenAs = (DWORD*) JV_GetNotepadOpenAsAddr(procBaseAddr, &winVer, hostArch);
    if (_g_ftOpenAs == NULL)
        return FALSE; // Non supported OS

    // g_ftOpenAs is in .data segment, which is Read/Write.
    // Unable to write? it must be wrong address
    // -1 : New file
    //  0 : Opened with ANSI
    //  1 : Opened with UTF16_LE
    //  2 : Opened with UTF16_BE
    //  3 : Opened with UTF8
    if (*_g_ftOpenAs == 0xFFFFFFFF) // default value - it's new file, not opened nor saved
        *_g_ftOpenAs = 0x03;

    return TRUE;
}

BOOL JV_IsThisProcessNotepad()
{
    // %windir%\system32\notepad.exe
    WCHAR procPath[MAX_PATH];
    WCHAR cmpPath[MAX_PATH];
    WCHAR winPath[MAX_BUF_LEN];
    GetModuleFileName(NULL, procPath, sizeof(procPath));
    procPath[MAX_PATH-1] = '\0'; // For Win XP

    GetEnvironmentVariableW(L"windir", winPath, sizeof(winPath));
    StringCbPrintfW(cmpPath, sizeof(cmpPath), L"%s\\system32\\notepad.exe", winPath);
    if (StrCmpIW(procPath, cmpPath) == 0)
        return TRUE;
    else
    {
        StringCbPrintfW(cmpPath, sizeof(cmpPath), L"%s\\notepad.exe", winPath);
        if (StrCmpIW(procPath, cmpPath) == 0)
            return TRUE;
    }

    return FALSE;
}

// Get BaseAddress of this Process
BYTE* JV_GetBaseAddress(const DWORD dwPID)
{
    HANDLE hModule = INVALID_HANDLE_VALUE;
    MODULEENTRY32W me;
    void* procBaseAddr = (void*) 0;

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
		if (me.th32ProcessID == dwPID)
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

// Provide VA from RVA
BYTE* JV_GetNotepadOpenAsAddr(BYTE* baseAddr, const JV_WIN_VER* winVer, const DWORD hostArch)
{
    if (hostArch == 32)
    {
        // XP x86 SP3
        // _g_ftOpenAS = Base + 0x9030
        // _g_ftSaveAS = Base + 0xA528
        if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 5, 1, 2600, 5512))
            return baseAddr + 0x9030;
        // Vista x86 SP2
        // _g_ftOpenAS = Base + 0xA00C
        // _g_ftSaveAS = Base + 0xC000
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 0, 6001, 18000))
            return baseAddr + 0xA00C;
        // 7 x86 SP1 (Tested)
        // _g_ftOpenAS = Base + 0xC00C
        // _g_ftSaveAS = Base + 0xE040
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 1, 7600, 16385))
            return baseAddr + 0xC00C;
        // 8 x86 SP2
        // _g_ftOpenAS = Base + 0x1D008
        // _g_ftSaveAS = Base + 0x1F364
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 2, 9200, 16384))
            return baseAddr + 0x1D008;
        // 8.1 x86 Update 1
        // _g_ftOpenAS = Base + 0x17008
        // _g_ftSaveAS = Base + 0x19304
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 3, 9600, 17415))
            return baseAddr + 0x17008;
        // 10.0.10240 x86
        // _g_ftOpenAS = Base + 0x17108
        // _g_ftSaveAS = Base + 0x19260
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 10, 0, 10240, 16384))
            return baseAddr + 0x17108;
        // 10 x86 (10.0.10586, 10.0.10240)
        // _g_ftOpenAS = Base + 0x1C138
        // _g_ftSaveAS = Base + 0x1E400
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 10, 0, 10586, 0))
            return baseAddr + 0x1C138;
    }
    else if (hostArch == 64)
    {
        // Vista x64 SP2
        // _g_ftOpenAS = Base + 0x10558
        // _g_ftSaveAS = Base + 0x120B0
        if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 0, 6001, 18000))
            return baseAddr + 0x10558;
        // 7 x64 SP1
        // _g_ftOpenAS = Base + 0x10088
        // _g_ftSaveAS = Base + 0x12720
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 1, 7600, 16385))
            return baseAddr + 0x10088;
        // 8 x64
        // _g_ftOpenAS = Base + 0x1F00C
        // _g_ftSaveAS = Base + 0x21AE8
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 2, 9200, 16384))
            return baseAddr + 0x1F00C;
        // 8.1 x64 Update 1
        // _g_ftOpenAS = Base + 0x1A00C
        // _g_ftSaveAS = Base + 0x1C848
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 6, 3, 9600, 17415))
            return baseAddr + 0x1A00C;
        // 10.0.10240 x64
        // _g_ftOpenAS = Base + 0x1B220
        // _g_ftSaveAS = Base + 0x1D7A0
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 10, 0, 10240, 16384))
            return baseAddr + 0x1B220;
        // 10.0.10586 x64 (Tested)
        // _g_ftOpenAS = Base + 0x22260
        // _g_ftSaveAS = Base + 0x24A00
        else if (JV_CompareWinVer(winVer, JV_CMP_E, JV_CMP_L3, 10, 0, 10586, 0))
            return baseAddr + 0x22260;
    }

    return NULL;
}


/// Hook CreateProcessA, CreateProcessW (Non-Notepad)
BOOL JV_HookCreateProcess()
{
    // Hook CreateProcessA, CreateProcessW
    if (MH_Initialize() != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_Initialize() failed\n\n");
        return FALSE;
    }

    if (MH_CreateHook((LPVOID) &CreateProcessA, (LPVOID) &MyCreateProcessA, (LPVOID*) &fpCreateProcessA) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_CreateHook(&CreateProcessA) failed\n\n");
        return FALSE;
    }
    if (MH_CreateHook((LPVOID) &CreateProcessW, (LPVOID) &MyCreateProcessW, (LPVOID*) &fpCreateProcessW) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_CreateHook(&CreateProcessW) failed\n\n");
        return FALSE;
    }

    if (MH_EnableHook((LPVOID) &CreateProcessA) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_EnableHook(&CreateProcessW) failed\n\n");
        return FALSE;
    }
    if (MH_EnableHook((LPVOID) &CreateProcessW) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_EnableHook(&CreateProcessW) failed\n\n");
        return FALSE;
    }

    return TRUE;
}

BOOL JV_UnHookCreateProcess()
{
    // Unhook CreateProcessA, CreateProcessW
    if (MH_DisableHook((LPVOID) &CreateProcessA) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_DisableHook(&CreateProcessW) failed\n\n");
        return FALSE;
    }
    if (MH_DisableHook((LPVOID) &CreateProcessW) != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_DisableHook(&CreateProcessW) failed\n\n");
        return FALSE;
    }

    if (MH_Uninitialize() != MH_OK)
    {
        fprintf(stderr, "[ERR] MH_Uninitialize() failed\n\n");
        return FALSE;
    }

    return TRUE;
}

BOOL JV_InjectDllByHandle(const HANDLE hProcess, const WCHAR *szDllPath)
{
	HANDLE hThread;
	JV_WIN_VER winVer;
	fp_NtCreateThreadEx_t fp_NtCreateThreadEx = NULL;
	LPTHREAD_START_ROUTINE remoteThreadProc = NULL;
	DWORD dwBufSize = 0;
	void* remoteProcBuf = NULL;

	if (!hProcess)
		return FALSE;
	JV_GetHostVer(&winVer);

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

	// WaitForSingleObject(hThread, INFINITE);
	// VirtualFreeEx(hProcess, remoteProcBuf, 0, MEM_RELEASE);
	if (!hThread)
		return FALSE;

	CloseHandle(hProcess);
	CloseHandle(hThread);

	return TRUE;
}


/// Get Host Windows' Version/Architecture Information
BOOL JV_GetHostVer(JV_WIN_VER* winVer)
{
    DWORD fileVerBufSize = 0;
    UINT fileVerQueryValueSize = 0;
    VOID* fileVerBuf = NULL;
    VS_FIXEDFILEINFO* fileVerQueryValue = NULL;

    fileVerBufSize = GetFileVersionInfoSizeW(L"kernel32.dll", NULL);
    if (!fileVerBufSize)
	{
		fprintf(stderr, "[ERR] GetFileVersionInfoSizeW failed\nError Code : %lu\n\n", GetLastError());
		return FALSE;
	}
    fileVerBuf = (LPVOID) malloc(fileVerBufSize);
    GetFileVersionInfoW(L"kernel32.dll", 0, fileVerBufSize, fileVerBuf);
    VerQueryValueW(fileVerBuf, L"\\", (LPVOID*) &fileVerQueryValue, &fileVerQueryValueSize);
    winVer->major = fileVerQueryValue->dwFileVersionMS / 0x10000;
    winVer->minor = fileVerQueryValue->dwFileVersionMS % 0x10000;
    winVer->bMajor = fileVerQueryValue->dwFileVersionLS / 0x10000;
    winVer->bMinor = fileVerQueryValue->dwFileVersionLS % 0x10000;
    free(fileVerBuf);

    return TRUE;
}

DWORD JV_GetHostArch()
{
    BOOL isWOW64;
    SYSTEM_INFO sysInfo;

    GetNativeSystemInfo(&sysInfo);
    switch (sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_INTEL: // x86
        if (!GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"))
            return 32; // No WOW64, in fact it must be Windows XP SP1
        if (!IsWow64Process(GetCurrentProcess(), &isWOW64))
        {
            fprintf(stderr, "[ERR] IsWow64Process() failed\nError Code : %lu\n\n", GetLastError());
            return FALSE;
        }
        if (isWOW64)
            return 64;
        else
            return 32;
        break;
    case PROCESSOR_ARCHITECTURE_AMD64: // x64
        return 64;
        break;
    }

    return 0;
}

DWORD JV_GetProcArch()
{
    if (sizeof(void*) == 8)
        return 64;
    else if (sizeof(void*) == 4)
        return 32;
    return 0;
}

BOOL JV_CompareWinVer(const JV_WIN_VER* wv, const DWORD op, const DWORD effective, const WORD major, const WORD minor, const WORD bMajor, const WORD bMinor)
{
    BOOL result = FALSE;
    uint64_t op_wv = 0;
    uint64_t op_cmp = 0;

    if (!(1 <= effective && effective <= 4))
        return FALSE;

    switch (effective)
    {
    case 1:
        op_wv = wv->major;
        op_cmp = major;
        break;
    case 2:
        op_wv = (wv->major * JV_CMP_MUL_L1) + wv->minor;
        op_cmp = (major * JV_CMP_MUL_L1) + minor;
        break;
    case 3:
        op_wv = (wv->major * JV_CMP_MUL_L2) + (wv->minor * JV_CMP_MUL_L1) + wv->bMajor;
        op_cmp = (major * JV_CMP_MUL_L2) + (minor * JV_CMP_MUL_L1) + bMajor;
        break;
    case 4:
        op_wv = (wv->major * JV_CMP_MUL_L3) + (wv->minor * JV_CMP_MUL_L2) + (wv->bMajor * JV_CMP_MUL_L1) + wv->bMinor;
        op_cmp = (major * JV_CMP_MUL_L3) + (minor * JV_CMP_MUL_L2) + (bMajor * JV_CMP_MUL_L1) + bMinor;
        break;
    }

    switch (op)
    {
    case JV_CMP_L:
        if (op_cmp < op_wv)
            result = TRUE;
        break;
    case JV_CMP_LE:
        if (op_cmp <= op_wv)
            result = TRUE;
        break;
    case JV_CMP_E:
        if (op_cmp == op_wv)
            result = TRUE;
        break;
    case JV_CMP_GE:
        if (op_cmp >= op_wv)
            result = TRUE;
        break;
    case JV_CMP_G:
        if (op_cmp > op_wv)
            result = TRUE;
        break;
    }

    return result;
}


/// CreateProcessA, CreateProcessW Hooking for Non-Notepad
BOOL WINAPI MyCreateProcessA(
    LPCSTR                lpApplicationName,
	LPSTR                 lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCSTR                lpCurrentDirectory,
	LPSTARTUPINFO         lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL result;
    WCHAR modName[MAX_BUF_LEN];
    result = fpCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                            bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                            lpStartupInfo, lpProcessInformation);
    GetModuleFileNameW(g_hInstance, modName, sizeof(modName));
    if (result)
        JV_InjectDllByHandle(lpProcessInformation->hProcess, modName);
    return result;

}

BOOL WINAPI MyCreateProcessW(
	LPCWSTR                lpApplicationName,
	LPWSTR                 lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR                lpCurrentDirectory,
	LPSTARTUPINFO         lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
    BOOL result;
    WCHAR modName[MAX_BUF_LEN];
    result = fpCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes,
                            bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory,
                            lpStartupInfo, lpProcessInformation);
    GetModuleFileNameW(g_hInstance, modName, sizeof(modName));
    if (result)
        JV_InjectDllByHandle(lpProcessInformation->hProcess, modName);
    return result;
}

/// Use SetWindowsHookEx Injection Method
BOOL DLL_EXPORT JV_MessageHookStart()
{
	g_hProcHook = SetWindowsHookExW(WH_CBT, JV_CBTProc, g_hInstance, 0);
	if (!g_hProcHook)
        return FALSE;
    return TRUE;
}

BOOL DLL_EXPORT JV_MessageHookStop()
{
    if (g_hProcHook)
    {
        if (!UnhookWindowsHookEx(g_hProcHook))
        {
            g_hProcHook = NULL;
            return FALSE;
        }
        else
            return TRUE;
    }
    else
        return FALSE;
}

LRESULT CALLBACK JV_CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}
