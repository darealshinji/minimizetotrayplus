/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef ULONG_PTR
#define ULONG_PTR DWORD
#endif

#ifndef trayWin32WindowHider_h__
#define trayWin32WindowHider_h__

#include "trayInterface.h"
#include <windows.h>
#include <shellapi.h>

// {4EBCB545-53A2-4329-B40D-085627D9E963}
#define TRAY_WIN32_WINDOWHIDER_CID                      \
  { 0x4ebcb545, 0x53a2, 0x4329,                         \
    { 0xb4, 0xd, 0x8, 0x56, 0x27, 0xd9, 0xe9, 0x63 }    \
  }

#define TRAY_WIN32_WINDOWHIDER_CONTRACTID \
  "@codefisher.org/minimizetotray/window-hider;1"

class trayWin32WindowHider: public trayIWindowHider {
public:
	NS_DECL_ISUPPORTS
	NS_DECL_TRAYIWINDOWHIDER
	trayWin32WindowHider();

private:
	~trayWin32WindowHider();

protected:

	/* additional members */
	HWND* m_hwndList;
	int m_hwndListCount;
	static ATOM s_wndClass;

	/* window property constants */
	static const TCHAR* S_PROPINST;
	static const TCHAR* S_PROPPROC;

	/* attributes from idl */
	PRBool m_suppressed;

	/* the window watcher members */
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	WNDPROC m_OldProc;
	HWND m_hwnd;
	nsIDOMWindow* m_watchDomWindow;
};

#endif // trayWin32WindowHider_h__
