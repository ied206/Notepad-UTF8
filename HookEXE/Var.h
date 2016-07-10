#ifndef VAR_H_INCLUDED
#define VAR_H_INCLUDED

#define UNICODE
#define _UNICODE
#define OEMRESOURCE
#define _WIN32_WINNT 0x0501
#define NTDDI_VERSION NTDDI_WINXP
#define MAX_BUF_LEN 32767
#define JV_BUF_SIZE 512

// MACRO for DEBUG
#ifdef _DEBUG
	#define _DEBUG_CONSOLE
#endif

#define JV_WINDOW_NAME 		L"Joveler\'s Notepad-UTF8"
#define JV_CLASS_NAME		L"Joveler_Notepad_UTF8"
#define JV_SYSTRAY_TIP   	L"Notepad-UTF8"
#define JV_SYSTRAY_ID_ON	1
#define JV_SYSTRAY_ID_OFF	2
#define WM_APP_SYSTRAY_POPUP (WM_APP + 0x0001)

#define JV_ARG_QUIET_OFF	0
#define JV_ARG_QUIET_ON		1
#define JV_ARG_HELP_OFF		0
#define JV_ARG_HELP_ON		1

#define JV_STATE_TURN_OFF	0
#define JV_STATE_TURN_ON	1

typedef struct
{
    int quiet; // Do notification?
    int help; // help message
} JV_ARG;

#define JV_VER_MAJOR	1
#define JV_VER_MINOR	0
#define JV_WEB_SOURCE	L"https://github.com/ied206/Notepad-UTF8"
#define JV_WEB_BINARY	L"https://joveler.kr/project/notepad-utf8.html"

#define DLL_NAME_32 L"Notepad-UTF8_x86.dll"
#define DLL_NAME_64 L"Notepad-UTF8_x64.dll"

#endif // VAR_H_INCLUDED

