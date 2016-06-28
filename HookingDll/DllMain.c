#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "MinHook.h"

typedef WINBOOL (WINAPI *GETSAVEFILENAMEW)(LPOPENFILENAME);
// typedef UINT_PTR (CALLBACK WINAPI *OFNHOOKPROC)(HWND, UINT, WPARAM, LPARAM);;

// Pointer for calling original GetSaveFileNameW.
GETSAVEFILENAMEW fpGetSaveFileNameW = NULL;
LPOFNHOOKPROC fpSaveDialogHookProc = NULL;

UINT_PTR CALLBACK DetourSaveDialogHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	// 0x20 : ANSI, 0x21 : UTF-16 LE, 0x22 : UTF-16 BE, 0x23 : UTF-8
    return fpSaveDialogHookProc(hdlg, uiMsg, wParam, 0x23);
}

// Detour function which overrides GetSaveFileNameW.
BOOL WINAPI DetourGetSaveFileNameW(LPOPENFILENAME lpofn)
{
    MessageBoxW(NULL, L"Hooked", L"Hooked", MB_OK);
	fpSaveDialogHookProc = lpofn->lpfnHook;
	lpofn->lpfnHook = DetourSaveDialogHookProc;
	lpofn->Flags = lpofn->Flags & ~OFN_ENABLEHOOK;
    return fpGetSaveFileNameW(lpofn);
}

void GetDebugPrivilege()
{
    HANDLE hCurrent = GetCurrentProcess();
    HANDLE hToken;
    TOKEN_PRIVILEGES newState, prevState;
    DWORD retLen;
    LUID luid;

    OpenProcessToken(hCurrent, TOKEN_ADJUST_PRIVILEGES, &hToken);
    newState.PrivilegeCount = 1;
    newState.Privileges[0].Luid = luid;
    newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    AdjustTokenPrivileges(hToken, FALSE, &newState, sizeof(TOKEN_PRIVILEGES), &prevState, &retLen);
    CloseHandle(hToken);

}


extern DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    // DWORD* _g_ftSaveAs = (DWORD*) 0x0041E400;
    DWORD* _g_ftSaveAs = (DWORD*) 0x0140024A00;
    DWORD src = 3;
    DWORD oldFlag, newFlag;
    // uint8_t* p = (uint8_t*) 0x0140024A00;
    char stmp[128];
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            if (MH_Initialize() != MH_OK)
                exit(1);
            if (MH_CreateHook((LPVOID) &GetSaveFileNameW, (LPVOID) &DetourGetSaveFileNameW, (LPVOID*)(&fpGetSaveFileNameW)) != MH_OK)
                exit(1);
            if (MH_EnableHook((LPVOID) &GetSaveFileNameW) != MH_OK)
                exit(1);
            // return FALSE to fail DLL load
            GetDebugPrivilege();
            if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
            {
                 MessageBoxA(NULL, "Failed", "alert", MB_OK);
            }
            *_g_ftSaveAs = 3;
            // VirtualProtect(_g_ftSaveAs, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &oldFlag);
            // memcpy((void*)_g_ftSaveAs, &src, sizeof(DWORD));
            // VirtualProtect(_g_ftSaveAs, sizeof(DWORD), oldFlag, &newFlag);
            MessageBoxA(NULL, "Success", "alert", MB_OK);
            /*
            asm(
                ".intel_syntax noprefix;\n"
                "push esi;\n"
                "mov esi,0x0041E400;\n"
                "mov [esi],3;\n"
                "pop esi;\n"
            );
            */
            break;
        case DLL_PROCESS_DETACH:
            // detach from process
            if (MH_DisableHook((LPVOID) &GetSaveFileNameW) != MH_OK)
                exit(1);
            if (MH_Uninitialize() != MH_OK)
                exit(1);
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


