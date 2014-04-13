#include "trayLinuxWindowIcon.h"
#include "trayRoutine.h"
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <gdk/gdkx.h>

#include "nsMemory.h"

#include "nsIPrefService.h"
#include "nsIPrefBranch2.h"

#include "nsIArray.h"

NS_IMPL_ISUPPORTS1(trayLinuxWindowIcon, trayIWindowIcon)

void emptyMenu(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(widget);
}

void menuAction(GtkMenuItem *menuitem, trayIMenuItem* item) {
    item->Callback(NULL);
}

NS_IMETHODIMP fillMenu(GtkWidget* menu, trayICallback* callback) {
	nsresult rv;
	nsIArray* menuItems;
	rv = callback->Call(&menuItems);

	NS_ENSURE_SUCCESS(rv, rv);

    PRUint32 length, type;
    nsCOMPtr<trayIMenuItem> item;
    GtkWidget* menuItem;
    GtkWidget* subMenu;
    nsString label;
    menuItems->GetLength(&length);
    trayICallback* subCallback;
    for (PRUint32 i = 0; i < length; i++) {
        rv = menuItems->QueryElementAt(i, NS_GET_IID(trayIMenuItem), getter_AddRefs(item));
        NS_ENSURE_SUCCESS(rv, rv);
        item->MenuType(&type);
        if(type == trayIMenuItem::MENU_ITEM || type == trayIMenuItem::MENU) {
            item->Label(label);
            menuItem = gtk_menu_item_new_with_mnemonic(g_utf16_to_utf8(ToNewUnicode(label), -1, NULL, NULL, NULL));
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
            if(type == trayIMenuItem::MENU) {
            	subMenu = gtk_menu_new();
            	item->GetCallback(&subCallback);
            	fillMenu(subMenu, subCallback);
            	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
            } else {
            	g_signal_connect(G_OBJECT(menuItem), "activate", G_CALLBACK(menuAction), item);
            }
        } else if(type == trayIMenuItem::SEPARATOR) {
            menuItem = gtk_separator_menu_item_new();
            gtk_menu_append(menu, menuItem);
        }
    }
    return NS_OK;
}

void trayLinuxWindowIcon::buttonEvent(GdkEventButton *event) {

	if(this->m_initialized == FALSE) {
		return;
	}

	nsString eventName;
	switch (event->type) {
		case GDK_BUTTON_RELEASE: // use release, so that we don't duplicate events
			eventName = NS_LITERAL_STRING("TrayClick");
			break;
		case GDK_2BUTTON_PRESS:
			eventName = NS_LITERAL_STRING("TrayDblClick");
			break;
		default:
			return;
	}

	nsPoint pt((nscoord)(event->x + event->x_root), (nscoord)(event->y + event->y_root));
#define HASSTATE(x) (event->state & x ? PR_TRUE : PR_FALSE)
	PRBool result = DispatchMouseEvent(this->m_domWindow, 
            eventName, 
            event->button - 1, 
            pt, 
            HASSTATE(GDK_CONTROL_MASK),
			HASSTATE(GDK_MOD1_MASK), 
            HASSTATE(GDK_SHIFT_MASK));
#undef HASSTATE
    if(result == PR_FALSE) {
        // show the menu
    	gtk_container_foreach(GTK_CONTAINER(this->m_tray_menu), emptyMenu, NULL);
    	fillMenu(this->m_tray_menu, this->m_menu_callback);
        gtk_widget_show_all(this->m_tray_menu);
        /* the button is 0, because other wise submenu items must right click to be activated */
        gtk_menu_popup(GTK_MENU(this->m_tray_menu), NULL, NULL,
                gtk_status_icon_position_menu, this->m_systray_icon, 0, event->time);
    }
}

trayLinuxWindowIcon::trayLinuxWindowIcon() {
	/* member initializers and constructor code */
	this->m_systray_icon = NULL;
	this->m_initialized = FALSE;

	this->m_systray_icon = gtk_status_icon_new();
	gtk_status_icon_set_visible(this->m_systray_icon, FALSE);
    this->m_tray_menu = gtk_menu_new();
}

trayLinuxWindowIcon::~trayLinuxWindowIcon() {
	/* destructor code */
	gtk_status_icon_set_visible(this->m_systray_icon, FALSE);
	g_object_unref(this->m_systray_icon);
    g_object_unref(this->m_tray_menu);
	this->m_systray_icon = NULL;
	this->m_initialized = FALSE;
}

NS_IMETHODIMP trayLinuxWindowIcon::SetTooltip(const nsAString & aTitle) {
	char* title;
    if(this->m_systray_icon == NULL) {
        return NS_ERROR_FAILURE;
    }
	if (aTitle.IsEmpty()) {
		title = (char*) gtk_window_get_title(this->m_window);
	} else {
		title = ToNewUTF8String(aTitle);
	}
    gtk_status_icon_set_tooltip(this->m_systray_icon, title);
}

/* void setup (in nsIDOMWindow aDomWindow, in AString aTitle); */
NS_IMETHODIMP trayLinuxWindowIcon::Setup(nsIDOMWindow *aDomWindow,
		const nsAString & aTitle, trayICallback* aMenuCallback) {
	nsresult rv;

	NS_ENSURE_ARG_POINTER(aDomWindow);
    NS_ENSURE_ARG(aMenuCallback);
	NS_ENSURE_TRUE(FALSE == this->m_initialized, NS_ERROR_ALREADY_INITIALIZED);
	this->m_initialized = TRUE;
	this->m_domWindow = aDomWindow;
	this->m_menu_callback = aMenuCallback;

	nsCOMPtr<nsIBaseWindow> baseWindow;
	rv = GetBaseWindow(aDomWindow, getter_AddRefs(baseWindow));
	NS_ENSURE_SUCCESS(rv, rv);

	nativeWindow aNativeWindow;
	rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
	NS_ENSURE_SUCCESS(rv, rv);

	GdkWindow *gdk_win = gdk_window_get_toplevel((GdkWindow*) aNativeWindow);
	GtkWidget* widget;
	gdk_window_get_user_data(gdk_win, (gpointer*) &widget);
	GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
	this->m_native_window = gdk_win;

	if (GTK_WIDGET_TOPLEVEL(toplevel)) {
		this->m_window = (GtkWindow*) toplevel;
		this->m_window_icon = gtk_window_get_icon(this->m_window);
		if (this->m_window_icon) {
			gtk_status_icon_set_from_pixbuf(this->m_systray_icon,
					this->m_window_icon);
			char* title;
			if (aTitle.IsEmpty()) {
				title = (char*) gtk_window_get_title(this->m_window);
			} else {
				title = ToNewUTF8String(aTitle);
			}
			gtk_status_icon_set_tooltip(this->m_systray_icon, title);
			g_signal_connect(G_OBJECT(this->m_systray_icon), "button-press-event",
					G_CALLBACK(trayLinuxWindowIcon::gtkButtonEvent), this);
			g_signal_connect(G_OBJECT(this->m_systray_icon), "button-release-event",
					G_CALLBACK(trayLinuxWindowIcon::gtkButtonEvent), this);
		}
	} else {
		return NS_ERROR_FAILURE;
	}
	return NS_OK;
}

/* void showIcon (); */
NS_IMETHODIMP trayLinuxWindowIcon::ShowIcon() {
	if (this->m_initialized && this->m_systray_icon) {
		gtk_status_icon_set_visible(this->m_systray_icon, TRUE);
	}
	return NS_OK;
}

/* void hideIcon (); */
NS_IMETHODIMP trayLinuxWindowIcon::HideIcon() {
	if (this->m_initialized && this->m_systray_icon) {
		gtk_status_icon_set_visible(this->m_systray_icon, FALSE);
	}
	return NS_OK;
}
