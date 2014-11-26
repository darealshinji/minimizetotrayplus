#ifndef TRAYLINUXWINDOWICON_H_
#define TRAYLINUXWINDOWICON_H_

#include "trayInterface.h"
#include "nsCOMPtr.h"
#include <gtk/gtk.h>

// {B991324A-690D-11DE-AE68-001320848EBC}
#define TRAY_LINUX_WINDOWICON_CID                      \
  { 0xb991324a, 0x690d, 0x11de,                         \
	{ 0xae, 0x68, 0x00, 0x13, 0x20, 0x84, 0x8e, 0xbc }    \
  }

#define TRAY_LINUX_WINDOWICON_CONTRACTID \
	"@codefisher.org/minimizetotray/window-icon;1"

class trayLinuxWindowIcon: public trayIWindowIcon {
public:
	NS_DECL_ISUPPORTS
	NS_DECL_TRAYIWINDOWICON

	trayLinuxWindowIcon();

    static void gtkButtonEvent(GtkStatusIcon*, GdkEventButton *event, trayLinuxWindowIcon *self) {
		self->buttonEvent(event);
	}


private:
	~trayLinuxWindowIcon();

protected:
	/* additional members */
	GtkStatusIcon *m_systray_icon;
	GtkWindow *m_window;
	GdkWindow *m_native_window;
	nsIDOMWindow *m_domWindow;
	GdkPixbuf *m_window_icon;
    GtkWidget *m_tray_menu;
    nsCOMPtr<trayICallback> m_menu_callback;

	bool m_initialized;

    void buttonEvent(GdkEventButton *event);
};

#endif /* TRAYLINUXWINDOWICON_H_ */
