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

#endif // VAR_H_INCLUDED
