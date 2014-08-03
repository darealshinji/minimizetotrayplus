#ifndef trayToolkit_h__
#define trayToolkit_h__

#include "nspr.h"

#include <windows.h>
#include <shellapi.h>

/*
 * A small duplication of the code from nsToolkit.h
 * So we can work on Win9x (ANSI) and WinNT (Unicode)
 * Unfortunately, nsToolkit is too deep in Mozilla for us to use
 */

#if _MSC_VER < 1300
#define LONG_PTR LONG
#define GWLP_WNDPROC GWL_WNDPROC
#define GCLP_HICONSM GCL_HICONSM
#define GetClassLongPtr GetClassLong
#define GetWindowLongPtrA GetWindowLongA
#define SetWindowLongPtrA SetWindowLongA
#define GetWindowLongPtrW GetWindowLongW
#define SetWindowLongPtrW SetWindowLongW
#endif

typedef LONG_PTR (WINAPI *TT_GetWindowLongPtr)(HWND, int);
typedef LONG_PTR (WINAPI *TT_SetWindowLongPtr)(HWND, int, LONG_PTR);
typedef int (WINAPI *TT_GetWindowText) (HWND, LPWSTR, int);
typedef BOOL (WINAPI *TT_Shell_NotifyIcon)(DWORD, PNOTIFYICONDATAW);

class trayToolkit {
public:
	static void Init();

	static PRBool m_isInitialized;
	static PRBool m_isNT;

	static TT_GetWindowLongPtr pGetWindowLongPtr;
	static TT_SetWindowLongPtr pSetWindowLongPtr;
	static TT_GetWindowText pGetWindowText;
	static TT_Shell_NotifyIcon pShell_NotifyIcon;

	static int WINAPI GetWindowTextA(HWND, LPWSTR, int);
	static BOOL WINAPI Shell_NotifyIconA(DWORD, PNOTIFYICONDATAW);

};

#endif // trayToolkit_h__
