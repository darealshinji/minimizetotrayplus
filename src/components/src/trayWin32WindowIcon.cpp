/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include "trayWin32WindowIcon.h"
#include "trayToolkit.h"
#include "trayRoutine.h"

#include "nsMemory.h"
#include "nsCOMPtr.h"
#include "nsStringAPI.h"
#include "nsServiceManagerUtils.h"

#include "nsIDOMDocumentEvent.h"
#include "nsIDOMEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMMouseEvent.h"

#include "nsIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentView.h"
#include "nsIDOMAbstractView.h"

#ifndef MOZILLA_VERSION_20
#include "docshell/nsIWebNavigation.h"
#include "widget/nsIBaseWindow.h"
#include "content/nsIPrivateDOMEvent.h"
#else 
#include "nsIWebNavigation.h"
#include "nsIBaseWindow.h"
#include "nsIPrivateDOMEvent.h"
#endif

#include "nsIInterfaceRequestorUtils.h"
#include "nsIObserverService.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch2.h"
#include "nsIArray.h"

const TCHAR* trayWin32WindowIcon::S_PROPINST = TEXT(
		"_MOOK_TRAY_WINDOWICON_INST");
const TCHAR* trayWin32WindowIcon::S_PROPPROC = TEXT(
		"_MOOK_TRAY_WINDOWICON_PROC");

ATOM trayWin32WindowIcon::s_wndClass = NULL;

#define MK_ERROR_OFFSET       (0xE0000000 + (__LINE__ * 0x10000))

#define MK_ENSURE_NATIVE(res)                              \
	PR_BEGIN_MACRO                                           \
	NS_ENSURE_TRUE(0 != res || 0 == ::GetLastError(),      \
	::GetLastError() + MK_ERROR_OFFSET);                 \
	PR_END_MACRO

// this can be WM_USER + anything
#define WM_TRAYICON (WM_USER + 0x17b6)

NS_IMPL_ISUPPORTS1(trayWin32WindowIcon, trayIWindowIcon)

trayWin32WindowIcon::trayWin32WindowIcon()
{
	/* member initializers and constructor code */
	memset(&(this->m_IconData), 0, sizeof(NOTIFYICONDATAW));
	this->m_IconData.cbSize = sizeof(NOTIFYICONDATAW);
	this->m_IconData.uCallbackMessage = WM_TRAYICON;
	this->m_IconData.uID = 1;
	this->m_IconData.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;
	this->m_MonitorThread = NULL;
	this->m_ListenerWindow = NULL;
	this->m_hwnd = NULL;
	this->m_initialized = FALSE;
	this->m_hasIcon = FALSE;
	this->m_title = nsnull;
	trayToolkit::Init();
}

trayWin32WindowIcon::~trayWin32WindowIcon() {
	/* destructor code */
	if (this->m_hasIcon) {
		this->HideIcon();
	}
	this->m_hwnd = nsnull;

	nsMemory::Free(this->m_title);

	// the call may fail if there are more windows of the same type
	// that's fine, eventually the last one would succeed.
	BOOL windowClassUnregistered = ::UnregisterClass(
			(LPCTSTR) trayWin32WindowIcon::s_wndClass, ::GetModuleHandle(NULL));
	if (windowClassUnregistered)
		trayWin32WindowIcon::s_wndClass = NULL;

	this->MonitorTerminate();
}

NS_IMETHODIMP trayWin32WindowIcon::SetTooltip(const nsAString & aTitle) {
	if (0 == aTitle.Length()) {
		nsMemory::Free(this->m_title);
	} else {
		this->m_title = NS_StringCloneData(aTitle);
	}
	//this->SetIconTooltip((HWND) this->m_hwnd);
	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::SetIconTooltip(HWND hwnd) {
	if (this->m_title) {
		wcsncpy_s(this->m_IconData.szTip, this->m_title,
				sizeof(this->m_IconData.szTip)
						/ sizeof(this->m_IconData.szTip[0]));
	} else {
		// no title supplied, use the caption of the window
		::SetLastError(0);
		MK_ENSURE_NATIVE(trayToolkit::pGetWindowText(
						hwnd,
						this->m_IconData.szTip, 64)
		);
	}
}

/* void setup (
 *   in PRUint32 aCount,
 *   [array, size_is (aCount)] in nsIBaseWindow aBaseWindows,
 *   in trayIWindowWatcherCallback aCallback);
 */
NS_IMETHODIMP trayWin32WindowIcon::Setup(nsIDOMWindow *aDomWindow,
		const nsAString& aTitle, trayICallback *aMenuCallback) {
	nsresult rv;
	this->m_hasIcon = FALSE;

	NS_ENSURE_ARG_POINTER(aDomWindow);
	NS_ENSURE_ARG(aMenuCallback);
	NS_ENSURE_TRUE(FALSE == this->m_initialized, NS_ERROR_ALREADY_INITIALIZED);
	this->m_initialized = TRUE;
	this->m_domWindow = aDomWindow;
	this->m_menu_callback = aMenuCallback;
	this->m_menu = CreatePopupMenu();

	nsCOMPtr<nsIBaseWindow> baseWindow;
	rv = GetBaseWindow(aDomWindow, getter_AddRefs(baseWindow));
	NS_ENSURE_SUCCESS(rv, rv);

	nativeWindow aNativeWindow;
	rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
	NS_ENSURE_SUCCESS(rv, rv);

	this->m_hwnd = reinterpret_cast<HWND> (aNativeWindow);
	NS_ENSURE_ARG_POINTER(this->m_hwnd);

	if (0 == aTitle.Length()) {
		nsMemory::Free(this->m_title);
	} else {
		this->m_title = NS_StringCloneData(aTitle);
	}

	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::HideIcon() {
	NS_ENSURE_TRUE(TRUE == this->m_initialized, NS_ERROR_NOT_INITIALIZED);

	if (this->m_hasIcon == FALSE) {
		return NS_OK;
	}
	this->m_hasIcon = FALSE;
	trayToolkit::pShell_NotifyIcon(NIM_DELETE, &this->m_IconData);
	::DestroyWindow(this->m_ListenerWindow);
	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::ShowIcon() {
	NS_ENSURE_TRUE(TRUE == this->m_initialized, NS_ERROR_NOT_INITIALIZED);

	if (this->m_hasIcon == TRUE) {
		return NS_OK;
	}
	this->m_hasIcon = TRUE;
	nsresult rv;
	rv = this->CreateListenerWindow();
	NS_ENSURE_SUCCESS(rv, rv);

	rv = this->AddTrayIcon((HWND) this->m_hwnd);
	NS_ENSURE_SUCCESS(rv, rv);
	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::AddTrayIcon(HWND hwnd) {
	nsresult rv;

	// find the window icon
	rv = this->GetIcon(hwnd, this->m_IconData.hIcon);
	NS_ENSURE_SUCCESS(rv, rv);

	// get the title
	this->SetIconTooltip(hwnd);

	// add the icon
	this->m_IconData.hWnd = this->m_ListenerWindow;
	MK_ENSURE_NATIVE(trayToolkit::pShell_NotifyIcon(NIM_ADD, &this->m_IconData));

	// monitor explorer crashes
	this->MonitorSystemTray();

	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::GetIcon(HWND hwnd, HICON& result) {
	result = (HICON) ::SendMessage(hwnd, WM_GETICON, ICON_SMALL, NULL);
	if (!result) {
		// can't find icon; try GetClassLong
		result = (HICON) ::GetClassLongPtr(hwnd, GCLP_HICONSM);
	}
	if (!result) {
		// still no icon - use generic windows icon
		result = ::LoadIcon(NULL, IDI_WINLOGO);
	}
	if (!result) {
		return NS_ERROR_FAILURE;
	}
	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowIcon::CreateListenerWindow() {
	::SetLastError(0);
	HINSTANCE hInst = ::GetModuleHandle(NULL);
	MK_ENSURE_NATIVE(hInst);

	if (!trayWin32WindowIcon::s_wndClass) {
		WNDCLASS wndClassDef;
		wndClassDef.style = CS_NOCLOSE | CS_GLOBALCLASS;
		wndClassDef.lpfnWndProc = trayWin32WindowIcon::WindowProc;
		wndClassDef.cbClsExtra = 0;
		wndClassDef.cbWndExtra = 0;
		wndClassDef.hInstance = hInst;
		wndClassDef.hIcon = NULL;
		wndClassDef.hCursor = NULL;
		wndClassDef.hbrBackground = NULL;
		wndClassDef.lpszMenuName = NULL;
		wndClassDef.lpszClassName = TEXT("MinimizeToTray:MessageWindowClass");

		trayWin32WindowIcon::s_wndClass = ::RegisterClass(&wndClassDef);
		MK_ENSURE_NATIVE(trayWin32WindowIcon::s_wndClass)
		;
	}

	this->m_ListenerWindow = ::CreateWindow(
			(LPCTSTR) trayWin32WindowIcon::s_wndClass, //class
			TEXT("MinimizeToTray:MessageWindow"), //caption
			WS_MINIMIZE, //style
			CW_USEDEFAULT, //x
			CW_USEDEFAULT, //y
			CW_USEDEFAULT, //width
			CW_USEDEFAULT, //height
			::GetDesktopWindow(), //parent
			NULL, //menu
			hInst, //hInst
			NULL); //param

	if (!this->m_ListenerWindow) {
		if (::UnregisterClass((LPCTSTR) trayWin32WindowIcon::s_wndClass, hInst)) {
			trayWin32WindowIcon::s_wndClass = NULL;
		}
		MK_ENSURE_NATIVE(this->m_ListenerWindow)
		;
	}
	MK_ENSURE_NATIVE(::SetProp(
					this->m_ListenerWindow,
					S_PROPINST,
					(HANDLE)this)
	);

	return NS_OK;
}

NS_IMETHODIMP fillMenu(HMENU menu, trayICallback* callback) {
	nsresult rv;
	nsIArray* menuItems;
	rv = callback->Call(&menuItems);

	NS_ENSURE_SUCCESS(rv, rv);

	PRUint32 length, type;
	nsCOMPtr<trayIMenuItem> item;
	nsString label;
	menuItems->GetLength(&length);
	trayICallback* subCallback;
	for (PRUint32 i = 0; i < length; i++) {
		rv = menuItems->QueryElementAt(i, NS_GET_IID(trayIMenuItem), getter_AddRefs(item));
		NS_ENSURE_SUCCESS(rv, rv);
		item->MenuType(&type);
		if(type == trayIMenuItem::MENU_ITEM) {
			item->Label(label);
			AppendMenu(menu, MF_STRING, 9001, (LPCSTR)ToNewUTF8String(label));
		} else if(type == trayIMenuItem::MENU) {
			item->Label(label);
			HMENU subMenu = CreatePopupMenu();
        	item->GetCallback(&subCallback);
        	fillMenu(subMenu, subCallback);
			AppendMenu(menu, MF_POPUP, (UINT_PTR)subMenu, (LPCSTR)ToNewUTF8String(label));
		} else if(type == trayIMenuItem::SEPARATOR) {
			AppendMenu(menu, MF_SEPARATOR, 0, 0);
		}
	}
}

LRESULT CALLBACK trayWin32WindowIcon::WindowProc(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	trayWin32WindowIcon* self =
			(trayWin32WindowIcon*) GetProp(hwnd, S_PROPINST);

	nsString eventName;
	PRUint16 button = 0;

	switch (uMsg) {
	case WM_TRAYICON:
		break;
	case WM_CREATE:
		return 0;
	case WM_NCCREATE:
		return true;
	}

	if (uMsg == WM_TRAYICON) {
		switch (LOWORD(lParam)) {
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_CONTEXTMENU:
		case NIN_KEYSELECT:
			eventName = NS_LITERAL_STRING("TrayClick");
			break;
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			eventName = NS_LITERAL_STRING("TrayDblClick");
			break;
		}

		switch (LOWORD(lParam)) {
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
			button = 0;
			break;
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			button = 1;
			break;
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_CONTEXTMENU:
		case NIN_KEYSELECT:
			button = 2;
			break;
		}
		if (eventName.IsEmpty() == PR_FALSE) {
			POINT wpt;
			if (GetCursorPos(&wpt) == TRUE) {
				nsPoint pt((nscoord) wpt.x, (nscoord) wpt.y);
				PRBool ctrlKey = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
				PRBool altKey = (::GetKeyState(VK_MENU) & 0x8000) != 0;
				PRBool shiftKey = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

				// SFW/PM is a win32 hack, so that the context menu is hidden when loosing focus.
				::SetForegroundWindow(hwnd);
				PRBool result = DispatchMouseEvent(self->m_domWindow, eventName, button, pt,
						ctrlKey, altKey, shiftKey);
				::PostMessage(self->m_hwnd, WM_NULL, 0, 0L);
				if(result == PR_FALSE) {
					// show the menu
					unsigned int clicked;
					while(GetMenuItemCount(self->m_menu) > 0) {
						DeleteMenu(self->m_menu, 0, MF_BYPOSITION);
					}
					fillMenu(self->m_menu, self->m_menu_callback);
					clicked = TrackPopupMenu(self->m_menu, TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, wpt.x, wpt.y, 0, self->m_hwnd, 0);
				}
				return FALSE;
			}
		}
	}

	return ::CallWindowProc(DefWindowProc, hwnd, uMsg, wParam, lParam);
}

void trayWin32WindowIcon::MonitorSystemTray() {
	if (this->m_MonitorThread) {
		// old monitor exists already; ask it to die
		this->MonitorTerminate();
	}

	DWORD dThreadID;
	this->m_MonitorThread = ::CreateThread(NULL, 0,
			trayWin32WindowIcon::MonitorProc, this, 0, &dThreadID);
}

void trayWin32WindowIcon::MonitorTerminate() {
	::QueueUserAPC(trayWin32WindowIcon::MonitorWakeupAPC,
			this->m_MonitorThread, 0);
	this->m_MonitorThread = NULL;
}

DWORD WINAPI trayWin32WindowIcon::MonitorProc(LPVOID inst) {

	// first, find the systray window (taskbar)
	HWND hSysTray = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);

	// and the process that holds it - we monitor that.
	DWORD pidExplorer;
	::GetWindowThreadProcessId(hSysTray, &pidExplorer);
	HANDLE hExplorer = ::OpenProcess(SYNCHRONIZE, FALSE, pidExplorer);

	// okay, we can now wait for that process
	DWORD dReason;
	dReason = ::WaitForSingleObjectEx(hExplorer, INFINITE, TRUE);

	if (WAIT_IO_COMPLETION == dReason) {
		// we were asked to stop, so don't try to re-add the icon
		return 0;
	}

	// we didn't request to stop - explorer had died

	// re-add the tray icon
	// (this will restart a new monitor thread)
	trayWin32WindowIcon* self = (trayWin32WindowIcon*) inst;
	self->AddTrayIcon(self->m_hwnd);

	return 0;
}

VOID CALLBACK trayWin32WindowIcon::MonitorWakeupAPC(ULONG_PTR) {
	// this is a stub - all we really need to do is to let the
	// monitor thread know that we want it to quit; we don't
	// actually need it to do any callbacks
}
