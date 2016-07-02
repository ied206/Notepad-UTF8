#ifndef HOST_H_INCLUDED
#define HOST_H_INCLUDED

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

/// Get Host Windows' Version/Architecture Information
BOOL JV_GetHostVer(JV_WIN_VER* winVer);
DWORD JV_GetHostArch();
DWORD JV_GetProcArch();
BOOL JV_CompareWinVer(const JV_WIN_VER* wv, const DWORD op, const DWORD effective, const WORD major, const WORD minor, const WORD bMajor, const WORD bMinor);

#endif // HOST_H_INCLUDED
