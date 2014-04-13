/*
 * impliments the watch function of the trayIWindowHider
 */

#include "nsCOMPtr.h"
#include "nsStringAPI.h"
#include "nsServiceManagerUtils.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIObserverService.h"

#include "trayToolkit.h"
#include "trayRoutine.h"
#include "trayWin32WindowHider.h"
#include "nsMemory.h"
#include "nsIDOMWindow.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch2.h"

#ifndef MOZILLA_VERSION_20
#include "docshell/nsIWebNavigation.h"
#include "widget/nsIBaseWindow.h"
#else 
#include "nsIWebNavigation.h"
#include "nsIBaseWindow.h"
#endif

#define MK_ERROR_OFFSET       (0xD0000000 + (__LINE__ * 0x10000))

#define MK_ENSURE_NATIVE(res)                              \
	PR_BEGIN_MACRO                                           \
	NS_ENSURE_TRUE(0 != res || 0 == ::GetLastError(),      \
	::GetLastError() + MK_ERROR_OFFSET);                 \
	PR_END_MACRO

/* boolean windowActive(); */
NS_IMETHODIMP trayWin32WindowHider::WindowActive(PRBool *active) {
	if(this->m_hwnd == ::GetForegroundWindow()) {
		*active = PR_TRUE;
	} else {
		*active = PR_FALSE;
	}
	return NS_OK;
}

/* void watch (in nsIDOMWindow aDOMWindow); */
NS_IMETHODIMP trayWin32WindowHider::Watch(nsIDOMWindow *aDomWindow) {
	nsresult rv;

	NS_ENSURE_ARG(aDomWindow);
	this->m_watchDomWindow = aDomWindow;

	nsCOMPtr<nsIBaseWindow> baseWindow;
	rv = GetBaseWindow(aDomWindow, getter_AddRefs(baseWindow));
	NS_ENSURE_SUCCESS(rv, rv);

	nativeWindow aNativeWindow;
	rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
	NS_ENSURE_SUCCESS(rv, rv);

	this->m_hwnd = reinterpret_cast<HWND> (aNativeWindow);
	NS_ENSURE_ARG_POINTER(this->m_hwnd);

	// subclass the window (need to intercept WM_TRAYICON)
	::SetLastError(0);

	if (::GetProp(this->m_hwnd, S_PROPINST)) {
		return NS_ERROR_ALREADY_INITIALIZED;
	}
	MK_ENSURE_NATIVE(::SetProp(this->m_hwnd, S_PROPINST, (HANDLE)this));
	this->m_OldProc = (WNDPROC) trayToolkit::pGetWindowLongPtr(this->m_hwnd,
			GWLP_WNDPROC);
	::SetLastError(0);
	MK_ENSURE_NATIVE(trayToolkit::pSetWindowLongPtr(
					this->m_hwnd,
					GWLP_WNDPROC,
					(LONG_PTR)trayWin32WindowHider::WindowProc
			));

	return NS_OK;
}

LRESULT CALLBACK trayWin32WindowHider::WindowProc(HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	WNDPROC proc;
	trayWin32WindowHider* self = (trayWin32WindowHider*) GetProp(hwnd,
			S_PROPINST);
	bool handled = true;
	if (self && self->m_watchDomWindow && self->m_OldProc) {
		proc = self->m_OldProc;
	} else {
		// property not found - we don't exist anymore
		// use the original window proc
		proc = (WNDPROC) GetProp(hwnd, S_PROPPROC);
		if (!proc)
			// can't find the right process... skip over to the default
			// (this will, at the minimum, totally break the app)
			proc = DefWindowProc;
		handled = false;
	}

	enum {
		EVENT_NONE,
		EVENT_MOUSEDOWN_LEFT,
		EVENT_MOUSEDOWN_RIGHT,
		EVENT_MOUSEDOWN_MIDDLE,
		EVENT_WINDOW_CLOSE,
		EVENT_WINDOW_ACTIVATE,
		EVENT_WINDOW_ACTIVATING,
		EVENT_WINDOW_MINIMIZE,
		EVENT_WINDOW_POSCHANGED,
		EVENT_WINDOW_POSCHANGING
	} eventType;

	enum {
		PARAM_NONE, PARAM_MOUSE_MINIMIZE, PARAM_MOUSE_CLOSE
	} eventParam;

	WINDOWPOS* pwp;

	if (handled) {
		switch (uMsg) {
		case WM_WINDOWPOSCHANGING:
			pwp = (WINDOWPOS*) lParam;
			if (pwp->flags & SWP_SHOWWINDOW || pwp->flags & SWP_DRAWFRAME) {
				eventType = EVENT_WINDOW_ACTIVATING;
			} else {
				eventType = EVENT_WINDOW_POSCHANGING;
			}
			break;
			//these are the min, max, and close buttons for
		case WM_WINDOWPOSCHANGED:
			eventType = EVENT_WINDOW_POSCHANGED;
			break;
		case WM_NCLBUTTONDOWN:
			eventType = EVENT_MOUSEDOWN_LEFT;
			switch (wParam) {
			case HTCLOSE:
				eventParam = PARAM_MOUSE_CLOSE;
				break;
			default:
				handled = false;
			}
			break;
		case WM_NCRBUTTONDOWN:
			eventType = EVENT_MOUSEDOWN_RIGHT;
			switch (wParam) {
			case HTMINBUTTON: //HTREDUCE is the same value
				eventParam = PARAM_MOUSE_MINIMIZE;
				break;
			default:
				handled = false;
			}
			break;
		case WM_NCMBUTTONDOWN:
			eventType = EVENT_MOUSEDOWN_MIDDLE;
			switch (wParam) {
			case HTMINBUTTON:
				eventParam = PARAM_MOUSE_MINIMIZE;
				break;
			default:
				handled = false;
			}
			break;
		case WM_ACTIVATE:
			switch (LOWORD(wParam)) {
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				// window is being activated
				eventType = EVENT_WINDOW_ACTIVATE;
				break;
			default:
				handled = false;
			}
			break;
		case WM_SIZE:
			switch (wParam) {
			case SIZE_MINIMIZED:
				eventType = EVENT_WINDOW_MINIMIZE;
				break;
			default:
				// we're resizing, but not minimized, so some part must be visible
				eventType = EVENT_WINDOW_ACTIVATE;
				break;
			}
			break;
		case WM_SYSCOMMAND:
			switch (wParam) {
			case SC_CLOSE:
				//The user has clicked on the top left window icon and selected close...
				//Or the user typed Alt+F4.  
				eventType = EVENT_WINDOW_CLOSE;
				break;
			case SC_MINIMIZE:
				eventType = EVENT_WINDOW_MINIMIZE;
				//LOGGER((PRUnichar*)L"Minimize command was sent.");
				break;
			default:
				handled = false;
			}
			break;
		default:
			handled = false;
			break;
		}
	}

	if (handled) {

		PRBool ctrlKey = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		//PRBool altKey = (::GetKeyState(VK_MENU) & 0x8000) != 0;
		//PRBool shiftKey = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

		nsString eventName;
		nsCOMPtr<nsIPrefBranch2> prefs(
				do_GetService( NS_PREFSERVICE_CONTRACTID));

		if (eventType == EVENT_WINDOW_ACTIVATING) {
			// the window is activating, and we are flaged as suppressed, so don't do it
			if (self->m_suppressed) {
				WINDOWPOS * pwp;
				pwp = (WINDOWPOS*) lParam;
				pwp->flags = (pwp->flags & (~SWP_SHOWWINDOW)) | SWP_HIDEWINDOW;
				lParam = (LPARAM) pwp;
			}
		} /*else if(eventType == EVENT_WINDOW_POSCHANGED) {
			WINDOWPLACEMENT pl;
			pl.length = sizeof(WINDOWPLACEMENT);
			::GetWindowPlacement(hwnd, &pl);

			PRBool trayOnMinimize = FALSE;
			prefs->GetBoolPref("extensions.minimizetotray.always",
					&trayOnMinimize);

			if (trayOnMinimize && pl.showCmd == SW_SHOWMINIMIZED) {
				LOGGER((PRUnichar*)L"Window is minimized");
				//pl.showCmd = SW_HIDE;
				::SetWindowPlacement(hwnd, &pl);
			}
		} */else {
			if (eventType == EVENT_WINDOW_MINIMIZE) {
				// window is trying to minimize normally
				PRBool trayOnMinimize = FALSE;
				prefs->GetBoolPref("extensions.minimizetotray.always",
						&trayOnMinimize);
				if (trayOnMinimize) {
					if (ctrlKey) {
						eventName = NS_LITERAL_STRING("TrayMinimizeAll");
					} else {
						eventName = NS_LITERAL_STRING("TrayMinimize");
					}
				}
			} else if (eventType == EVENT_WINDOW_ACTIVATE) {
				// this window is being forcibly shown,  but not when suppressed
				if (!self->m_suppressed) {
					eventName = NS_LITERAL_STRING("TrayRestore");
				}
			} else if (eventType == EVENT_WINDOW_CLOSE) {
				// see if the user wishes to minimize to the tray instead of
				// closing.
				// this should NOT fire if File->Exit is run by the user. It
				// should
				// fire if the user typed Alt+F4 to close the window. This does
				// NOT include
				// the event when the user clicked on the close button. (To make
				// things more confusing
				// I believe Mozilla 1.7.8 handled both Alt+F4 and the left
				// click on the close button here)
				PRBool trayOnClose = FALSE;
				prefs->GetBoolPref(
						"extensions.minimizetotray.minimize-on-close",
						&trayOnClose);
				if (trayOnClose) {
					eventName = NS_LITERAL_STRING("TrayClose");
				} else {
					eventName = NS_LITERAL_STRING("TrayTurboClose");
				}
			} else if (eventType == EVENT_MOUSEDOWN_RIGHT && eventParam
					== PARAM_MOUSE_MINIMIZE) {
				if (ctrlKey) {
					eventName = NS_LITERAL_STRING("TrayMinimizeAll");
				} else {
					eventName = NS_LITERAL_STRING("TrayMinimize");
				}
			} else if (eventType == EVENT_MOUSEDOWN_MIDDLE && eventParam
					== PARAM_MOUSE_MINIMIZE) {
				eventName = NS_LITERAL_STRING("TrayMinimizeAll");
			} else if (eventType == EVENT_MOUSEDOWN_LEFT && eventParam
					== PARAM_MOUSE_CLOSE) {
				// Handle when the user attempts to close the window by left
				// clicking on the close button.
				// This does NOT cover the Alt+F4 case of closing a window.
				PRBool trayOnClose = PR_FALSE;
				prefs->GetBoolPref(
						"extensions.minimizetotray.minimize-on-close",
						&trayOnClose);
				if (trayOnClose) {
					if (ctrlKey) {
						eventName = NS_LITERAL_STRING("TrayMinimizeAll");
					} else {
						eventName = NS_LITERAL_STRING("TrayClose");
					}
				} else {
					eventName = NS_LITERAL_STRING("TrayTurboClose");
				}
			}
			if (eventName.IsEmpty() == PR_FALSE) {
				PRBool notHandeled = PR_TRUE;
				DispatchEvent(self->m_watchDomWindow, eventName, &notHandeled);
				if (notHandeled == FALSE) {
					return FALSE;
				}
			}
		}
	}
	return CallWindowProc(proc, hwnd, uMsg, wParam, lParam);
}

