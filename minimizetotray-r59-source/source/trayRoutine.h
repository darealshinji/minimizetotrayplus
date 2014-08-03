#ifndef trayRoutine_h__
#define trayRoutine_h__

#define TRAY_DEBUG TRUE

#include "nsIInterfaceRequestorUtils.h"
#include "nsServiceManagerUtils.h"

#include "nsIDOMWindow.h"
#include "nsStringAPI.h"

#ifndef MOZILLA_VERSION_20
#include "docshell/nsIWebNavigation.h"
#include "widget/nsIBaseWindow.h"
#include "xpcom/nsIConsoleService.h"
#include "gfx/nsPoint.h"
#else
#include "nsIWebNavigation.h"
#include "nsIBaseWindow.h"
#include "nsIConsoleService.h"
#include "nsPoint.h"
#endif

NS_IMETHODIMP GetBaseWindow(nsIDOMWindow *aWindow, nsIBaseWindow **aBaseWindow);
NS_IMETHODIMP DispatchEvent(nsIDOMWindow *aWindow, const nsAString& aEventName,
		PRBool* notHandeled);
bool DispatchMouseEvent(nsIDOMWindow *aWindow,
		const nsAString& aEventName, PRUint16 aButton, nsPoint& pt,
		PRBool aCtrlKey, PRBool aAltKey, PRBool aShiftKey);

void LOGGER(const char* msg);

#endif
