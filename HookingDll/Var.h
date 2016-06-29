#ifndef VAR_H_INCLUDED
#define VAR_H_INCLUDED

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#define MAX_BUF_LEN 32767

#include <windows.h>

#define JV_CMP_L    1
#define JV_CMP_LE   2
#define JV_CMP_E    3
#define JV_CMP_GE   4
#define JV_CMP_G    5
#define JV_CMP_L1   1
#define JV_CMP_L2   2
#define JV_CMP_L3   3
#define JV_CMP_L4   4
#define JV_CMP_MUL_L1 0x10000
#define JV_CMP_MUL_L2 0x100000000
#define JV_CMP_MUL_L3 0x1000000000000

typedef struct
{
	WORD major;
	WORD minor;
	WORD bMajor;
	WORD bMinor;
} JV_WIN_VER;


typedef BOOL (WINAPI *CREATEPROCESSA) (
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

typedef BOOL (WINAPI *CREATEPROCESSW) (
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


#endif // VAR_H_INCLUDED


