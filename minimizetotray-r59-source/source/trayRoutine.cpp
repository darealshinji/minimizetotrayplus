#include "trayRoutine.h"
#include "stdio.h"

#include "nsIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIDOMDocumentView.h"
#include "nsIDOMAbstractView.h"

#include "nsIDOMDocumentEvent.h"
#include "nsIDOMEvent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMMouseEvent.h"

#include "nsServiceManagerUtils.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIConsoleService.h"

#ifndef MOZILLA_VERSION_20
#include "content/nsIPrivateDOMEvent.h"
#else
#include "nsIPrivateDOMEvent.h"
#endif

NS_IMETHODIMP GetBaseWindow(nsIDOMWindow *aWindow, nsIBaseWindow **aBaseWindow) {

	NS_ENSURE_ARG_POINTER(aWindow);

	NS_ENSURE_ARG_POINTER(aBaseWindow);
	nsresult rv;

	nsCOMPtr<nsIWebNavigation> webNav = do_GetInterface(aWindow, &rv);
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIBaseWindow> baseWindow = do_QueryInterface(webNav, &rv);
	NS_ENSURE_SUCCESS(rv, rv);
	*aBaseWindow = baseWindow;
	NS_IF_ADDREF(*aBaseWindow);
	return NS_OK;
}

NS_IMETHODIMP DispatchEvent(nsIDOMWindow *aWindow, const nsAString& aEventName,
		PRBool* notHandeled) {
	NS_ENSURE_ARG_POINTER(aWindow);
	nsresult rv;

	nsCOMPtr<nsIDOMDocument> domDocument;
	rv = aWindow->GetDocument(getter_AddRefs(domDocument));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIDOMDocumentEvent> docEvent(do_QueryInterface(domDocument));
	nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(domDocument));
	NS_ENSURE_TRUE(docEvent && target, NS_ERROR_INVALID_ARG);

	nsCOMPtr<nsIDOMEvent> event;
	rv = docEvent->CreateEvent(NS_LITERAL_STRING("Events"), getter_AddRefs(
			event));
	NS_ENSURE_SUCCESS(rv, rv);

	rv = event->InitEvent(aEventName, PR_TRUE, PR_TRUE);
	NS_ENSURE_SUCCESS(rv, rv);
	return target->DispatchEvent(event, notHandeled);
	return NS_OK;
}

void LOGGER(const char* msg) {
#ifdef TRAY_DEBUG
	nsString txt;
	NS_CStringToUTF16(nsCString(msg),NS_CSTRING_ENCODING_ASCII,txt);

	nsCOMPtr<nsIConsoleService> consoleService(do_GetService(NS_CONSOLESERVICE_CONTRACTID));
	nsIConsoleService* cs = consoleService;
	cs->LogStringMessage(txt.get());
#endif
}

// taken from MinTrayR - no point reinventing it
bool DispatchMouseEvent(nsIDOMWindow *aWindow,
		const nsAString& aEventName, PRUint16 aButton, nsPoint& pt,
		PRBool aCtrlKey, PRBool aAltKey, PRBool aShiftKey) {

	nsresult rv;

	nsCOMPtr<nsIDOMDocument> domDocument;
	rv = aWindow->GetDocument(getter_AddRefs(domDocument));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIDOMDocumentEvent> docEvent(do_QueryInterface(domDocument));
	nsCOMPtr<nsIDOMEventTarget> target(do_QueryInterface(domDocument));
	NS_ENSURE_TRUE(docEvent && target, NS_ERROR_INVALID_ARG);

	nsCOMPtr<nsIDOMDocumentView> docView(do_QueryInterface(domDocument, &rv));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIDOMAbstractView> abstractView;
	rv = docView->GetDefaultView(getter_AddRefs(abstractView));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIDOMEvent> event;
	rv = docEvent->CreateEvent(NS_LITERAL_STRING("MouseEvents"),
			getter_AddRefs(event));
	NS_ENSURE_SUCCESS(rv, rv);

	nsCOMPtr<nsIDOMMouseEvent> mouseEvent(do_QueryInterface(event, &rv));
	NS_ENSURE_SUCCESS(rv, rv);

	rv = mouseEvent->InitMouseEvent(aEventName, PR_FALSE, PR_TRUE,
			abstractView, 0, pt.x, pt.y, 0, 0, aCtrlKey, aAltKey, aShiftKey,
			PR_FALSE, aButton, target);

	NS_ENSURE_SUCCESS(rv, rv);

    PRBool result;
	target->DispatchEvent(mouseEvent, &result);
    return result;
}
