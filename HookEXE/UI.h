#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#define ID_ABOUT		2000
#define ID_HELP			2001
#define ID_HOMEPAGE		2010
#define ID_LICENSE		2011
#define ID_STATE_BANNER	2020
#define ID_STATE_INFO	2021
#define ID_TOGGLE		2022
#define ID_EXIT			2030

HWND JVUI_InitWindow(HINSTANCE hInstance);
void JVUI_GetSystemDPI();
BOOL JVUI_ShowPopupMenu(HWND hWnd, POINT *curpos, int wDefaultItem);
void JVUI_AddTrayIcon(HWND hWnd, UINT uID, UINT flag, UINT uCallbackMsg, LPCWSTR lpInfoStr);
void JVUI_DelTrayIcon(HWND hWnd, UINT uID);
void JVUI_WM_CLOSE(HWND hWnd, uint8_t postquit);
void JVUI_OpenHomepage(HWND hWnd);
void JVUI_OpenLicense(HWND hWnd);
void JVUI_PrintBanner(HWND hWnd);
void JVUI_PrintHelp(HWND hWnd);

#endif // UI_H_INCLUDED
