/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef ULONG_PTR
#define ULONG_PTR DWORD
#endif

#ifndef trayWin32WindowIcon_h__
#define trayWin32WindowIcon_h__

#include <windows.h>
#include <shellapi.h>

#include "trayInterface.h"

#include "nsCOMArray.h"
#include "nsCOMPtr.h"

#ifndef MOZILLA_VERSION_20
#include "xpcom/nsAutoPtr.h"
#else
#include "nsAutoPtr.h"
#endif

#include "nsXPCOMStrings.h"
#include "nsIObserver.h"

// {B991324A-690D-11DE-AE68-001320848EBC}
#define TRAY_WIN32_WINDOWICON_CID                      \
  { 0xb991324a, 0x690d, 0x11de,                         \
	{ 0xae, 0x68, 0x00, 0x13, 0x20, 0x84, 0x8e, 0xbc }    \
  }

#define TRAY_WIN32_WINDOWICON_CONTRACTID \
	"@codefisher.org/minimizetotray/window-icon;1"

class trayWin32WindowIcon: public trayIWindowIcon {
public:
	NS_DECL_ISUPPORTS
	NS_DECL_TRAYIWINDOWICON

	trayWin32WindowIcon();

private:
	~trayWin32WindowIcon();

protected:
	NS_IMETHOD AddTrayIcon(HWND hwnd);
	NS_IMETHOD GetIcon(HWND hwnd, HICON& result);
	NS_IMETHOD CreateListenerWindow();
	NS_IMETHOD SetIconTooltip(HWND hwnd);

	static LRESULT CALLBACK WindowProc(
			HWND hwnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam
	);

	/* additional members */
	nsCOMPtr<trayICallback> m_menu_callback;
	nsIDOMWindow *m_domWindow;
	NOTIFYICONDATAW m_IconData;
	HWND m_ListenerWindow;
	HWND m_hwnd;
	bool m_initialized;
	PRUnichar *m_title;
	static ATOM s_wndClass;
	bool m_hasIcon;
	HMENU m_menu;

	/* window property constants */
	static const TCHAR* S_PROPINST;
	static const TCHAR* S_PROPPROC;

	/* for monitoring when explorer dies */
	void MonitorSystemTray();
	void MonitorTerminate();
	HANDLE m_MonitorThread;
static DWORD WINAPI MonitorProc(LPVOID inst);
static VOID CALLBACK MonitorWakeupAPC(ULONG_PTR);

};

#endif // trayWin32WindowIcon_h__
