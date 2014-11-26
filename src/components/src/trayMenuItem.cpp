#include "trayMenuItem.h"
#include "nsIArray.h"

NS_IMPL_ISUPPORTS1(trayMenuItem, trayIMenuItem)

trayMenuItem::~trayMenuItem() {

}

NS_IMETHODIMP trayMenuItem::Setup(PRUint32 aMenuType, const nsAString & aLabel, trayICallback *aCallback) {
    NS_ENSURE_ARG(aMenuType);
    if(aMenuType == trayIMenuItem::MENU_ITEM || aMenuType == trayIMenuItem::MENU) {
        NS_ENSURE_ARG(aCallback);
    }

	this->m_menu_type = aMenuType;
    this->m_label = nsString(aLabel);
    this->m_callback = aCallback;
    return NS_OK;
}

/* PRUint32 menuType (); */
NS_IMETHODIMP trayMenuItem::MenuType(PRUint32 *aMenuType) {
    *aMenuType = this->m_menu_type;
    return NS_OK;
}

/* AString label (); */
NS_IMETHODIMP trayMenuItem::Label(nsAString &aLabel) {
    aLabel = this->m_label;
    return NS_OK;
}

/* GetCallback(trayICallback **_retval) */
NS_IMETHODIMP trayMenuItem::GetCallback(trayICallback **callback) {
	*callback = this->m_callback;
    return NS_OK;
}

/* trayICallback callback (); */
NS_IMETHODIMP trayMenuItem::Callback(nsIArray **value) {
	if(value == NULL) {
		nsIArray* array;
		this->m_callback->Call(&array);
	} else {
		this->m_callback->Call(value);
	}
    return NS_OK;
}
