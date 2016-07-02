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

#include "Host.h"

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
        if (op_wv < op_cmp)
            result = TRUE;
        break;
    case JV_CMP_LE:
        if (op_wv <= op_cmp)
            result = TRUE;
        break;
    case JV_CMP_E:
        if (op_wv == op_cmp)
            result = TRUE;
        break;
    case JV_CMP_GE:
        if (op_wv >= op_cmp)
            result = TRUE;
        break;
    case JV_CMP_G:
        if (op_wv > op_cmp)
            result = TRUE;
        break;
    }

    return result;
}
