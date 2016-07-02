#ifndef VAR_H_INCLUDED
#define VAR_H_INCLUDED

#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0501
#define MAX_BUF_LEN 32767

// -h api // default
// -h api -g
// -h msg
#define JV_ARG_METHOD_API	1
#define JV_ARG_METHOD_MSG	2
#define JV_ARG_HELP_OFF		0
#define JV_ARG_HELP_ON		1

typedef struct
{
    int method; // Hook method
    int help; // help message
} JV_ARG;

#define JV_VER_MAJOR	1
#define JV_VER_MINOR	0
#define JV_WEB_SOURCE	"https://github.com/ied206/NotepadUTF8"
#define JV_WEB_RELEASE	"https://joveler.kr"

#define DLL_NAME_32 L"NotepadUTF8_x86.dll"
#define DLL_NAME_64 L"NotepadUTF8_x64.dll"

#endif // VAR_H_INCLUDED

