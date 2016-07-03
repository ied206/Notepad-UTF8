#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#define ID_ABOUT		2000
#define ID_HELP			2001
#define ID_TOGGLE		2002
#define ID_STATE_BANNER	2003
#define ID_STATE_INFO	2004
#define ID_EXIT			2005

HWND JVUI_InitWindow(HINSTANCE hInstance);
BOOL JVUI_ShowPopupMenu(HWND hWnd, POINT *curpos, int wDefaultItem);
void JVUI_AddTrayIcon(HWND hWnd, UINT uID, UINT flag, UINT uCallbackMsg, LPCWSTR lpInfoStr);
void JVUI_DelTrayIcon(HWND hWnd, UINT uID);
void JVUI_WM_CLOSE(HWND hWnd, uint8_t postquit);
void JVUI_PrintHelp();

#endif // UI_H_INCLUDED
