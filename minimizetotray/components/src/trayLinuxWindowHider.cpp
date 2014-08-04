#include "trayLinuxWindowHider.h"
#include "trayRoutine.h"
#include "keyState.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <gdk/gdkx.h>

#include "nsIPrefService.h"
#include "nsIPrefBranch2.h"

/* Implementation file */
NS_IMPL_ISUPPORTS1(trayLinuxWindowHider, trayIWindowHider)

trayLinuxWindowHider::trayLinuxWindowHider() {
	/* member initializers and constructor code */
	this->m_suppressed = false;
	this->m_windowListCount = 0;
}

trayLinuxWindowHider::~trayLinuxWindowHider() {
	/* destructor code */
	this->Restore();
}

/* attribute PRUint32 state; */
NS_IMETHODIMP trayLinuxWindowHider::GetSuppressed(PRBool *aSuppressed) {
	*aSuppressed = this->m_suppressed;
	return NS_OK;
}
NS_IMETHODIMP trayLinuxWindowHider::SetSuppressed(PRBool aSuppressed) {
	this->m_suppressed = aSuppressed;
	return NS_OK;
}

NS_IMETHODIMP trayLinuxWindowHider::WindowActive(PRBool *active) {
	//gdk_window_hide(this->m_windowList[i]);
	GtkWidget* widget;
	gdk_window_get_user_data(this->m_watchedWindow, (gpointer*) &widget);

	if(gtk_window_is_active((GtkWindow*)gtk_widget_get_toplevel(widget)) == TRUE) {
		*active = PR_TRUE;
	} else {
		*active = PR_FALSE;
	}
	return NS_OK;
}

/* void watch (in nsIDOMWindow aDOMWindow); */
NS_IMETHODIMP trayLinuxWindowHider::Watch(nsIDOMWindow *aDomWindow) {
	nsresult rv;
	LOGGER("doing window watch function");
	NS_ENSURE_ARG_POINTER(aDomWindow);
	this->m_watchDomWindow = aDomWindow;

	nsCOMPtr<nsIBaseWindow> baseWindow;
	rv = GetBaseWindow(aDomWindow, getter_AddRefs(baseWindow));
	NS_ENSURE_SUCCESS(rv, rv);

	nativeWindow aNativeWindow;
	rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
	NS_ENSURE_SUCCESS(rv, rv);

	this->m_watchedWindow = gdk_window_get_toplevel((GdkWindow*) aNativeWindow);
	gdk_window_add_filter(this->m_watchedWindow, trayLinuxWindowHider::WindowFilter, this);

	return NS_OK;
}

/* void minimize (in PRUint32 aCount, [array, size_is (aCount)] in nsIDOMWindow aDOMWindows); */
NS_IMETHODIMP trayLinuxWindowHider::Minimize(PRUint32 aCount,
		nsIDOMWindow **aDOMWindows) {
	nsresult rv;
	PRUint32 i;

	NS_ENSURE_ARG(aCount);
	NS_ENSURE_ARG_POINTER(aDOMWindows);
	NS_ENSURE_TRUE(0 == this->m_windowListCount, NS_ERROR_ALREADY_INITIALIZED);
	this->m_windowListCount = aCount;

	this->m_windowList = new GdkWindow*[aCount];
	if (!this->m_windowList) {
		return NS_ERROR_OUT_OF_MEMORY;
	}

	for (i = 0; i < aCount; ++i) {
		nsCOMPtr<nsIBaseWindow> baseWindow;
		rv = GetBaseWindow(aDOMWindows[i], getter_AddRefs(baseWindow));
		NS_ENSURE_SUCCESS(rv, rv);

		nativeWindow aNativeWindow;
		rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
		NS_ENSURE_SUCCESS(rv, rv);

		this->m_windowList[i] = gdk_window_get_toplevel(
				(GdkWindow*) aNativeWindow);
		NS_ENSURE_ARG_POINTER(this->m_windowList[i]);
	}

	// everything worked, now hide the window
	for (i = 0; i < aCount; ++i) {
		//gdk_window_hide(this->m_windowList[i]);
	    GtkWidget* widget;
	    gdk_window_get_user_data(this->m_windowList[i], (gpointer*) &widget);

        gtk_widget_hide(gtk_widget_get_toplevel(widget));        
	}

	return NS_OK;
}

/* void restore (); */
NS_IMETHODIMP trayLinuxWindowHider::Restore() {
	//NS_ENSURE_TRUE(0 < this->m_windowListCount, NS_ERROR_NOT_INITIALIZED);

    this->m_suppressed = PR_FALSE;

	for (PRInt32 i = 0; i < this->m_windowListCount; ++i) {
        //gdk_window_show(this->m_windowList[i]);
	    GtkWidget* widget;
	    gdk_window_get_user_data(this->m_windowList[i], (gpointer*) &widget);

        gtk_window_present((GtkWindow*)gtk_widget_get_toplevel(widget));
	}

	delete[] this->m_windowList;
	this->m_windowList = nsnull;
	this->m_windowListCount = 0;

	return NS_OK;
}

// based on code from FireTray
Atom delete_window = XInternAtom(GDK_DISPLAY(), "WM_DELETE_WINDOW", False);

GdkFilterReturn trayLinuxWindowHider::WindowFilter(GdkXEvent *xevent,
		GdkEvent *event, gpointer data) {
	if (!data || !xevent) {
		return GDK_FILTER_CONTINUE;
	}
	XEvent *e = (XEvent *) xevent;
	trayLinuxWindowHider *self = (trayLinuxWindowHider *) data;

	if (!self) {
		return GDK_FILTER_CONTINUE;
	}

	enum {
		EVENT_NONE,
		EVENT_WINDOW_CLOSE,
		EVENT_WINDOW_ACTIVATE,
		EVENT_WINDOW_MINIMIZE
	} eventType;

	eventType = EVENT_NONE;


	if (self->m_suppressed) {
		// try pushing it back again.  This is ugly, but for now it works.
		// need to track down now to block the MapNotify, which happens
		// before the VisibilityNotify event.
		int i;
		if(e->xany.type == MapNotify || e->xany.type == VisibilityNotify ) {
			for (i = 0; i < self->m_windowListCount; ++i) {
				//gdk_window_hide(this->m_windowList[i]);
				GtkWidget* widget;
				gdk_window_get_user_data(self->m_windowList[i], (gpointer*) &widget);

				gtk_widget_hide(gtk_widget_get_toplevel(widget));
			}
		}
		return GDK_FILTER_REMOVE;
	}

	switch (e->xany.type) {
	case MapNotify:
		// the window was restored
		eventType = EVENT_WINDOW_ACTIVATE;
		break;
	case UnmapNotify:
		// we expect to create these ourself when hiding the window,
		// in which case the window count will not be zero.  If it is
		// zero means that something else did it and we might be interested
		if (self->m_windowListCount == 0) {
			eventType = EVENT_WINDOW_MINIMIZE;
		}
		break;
	case ClientMessage:
		if (e->xclient.data.l && (unsigned int) e->xclient.data.l[0]
				== delete_window) {
			// the user tried to close the window in some way
			eventType = EVENT_WINDOW_CLOSE;
		}
		break;
	case VisibilityNotify:
		// the window was restored
		eventType = EVENT_WINDOW_ACTIVATE;
		break;
	default:
		break;
	}

	nsString eventName;
	nsCOMPtr<nsIPrefBranch2> prefs(do_GetService( NS_PREFSERVICE_CONTRACTID));

	if (eventType == EVENT_WINDOW_CLOSE) {
		PRBool trayOnClose = PR_FALSE;
		prefs->GetBoolPref("extensions.minimizetotray.minimize-on-close",
				&trayOnClose);
		if (trayOnClose) {
			if (keyIsPressed(CTRL_KEY)) {
				eventName = NS_LITERAL_STRING("TrayMinimizeAll");
			} else {
				eventName = NS_LITERAL_STRING("TrayClose");
			}
		} else {
			eventName = NS_LITERAL_STRING("TrayTurboClose");
		}
	} else if (eventType == EVENT_WINDOW_ACTIVATE) {
		if (self->m_suppressed) {
			return GDK_FILTER_REMOVE;
		} else {
			eventName = NS_LITERAL_STRING("TrayRestore");
		}
	} else if (eventType == EVENT_WINDOW_MINIMIZE) {
		// window is trying to minimize normally
		PRBool trayOnMinimize = FALSE;
		prefs->GetBoolPref("extensions.minimizetotray.always", &trayOnMinimize);
		if (trayOnMinimize) {
			if (keyIsPressed(CTRL_KEY)) {
				eventName = NS_LITERAL_STRING("TrayMinimizeAll");
			} else {
				eventName = NS_LITERAL_STRING("TrayMinimize");
			}
		}
	}
	if (eventName.IsEmpty() == PR_FALSE) {
		PRBool notHandeled = PR_TRUE;
		DispatchEvent(self->m_watchDomWindow, eventName, &notHandeled);
		if (notHandeled == FALSE) {
			return GDK_FILTER_REMOVE; // it was processed so we will swollow it
		}
	}
	return GDK_FILTER_CONTINUE;
}
