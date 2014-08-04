#ifndef TRAYLINUXWINDOWHIDER_CPP_
#define TRAYLINUXWINDOWHIDER_CPP_

#include "trayInterface.h"
#include <gtk/gtk.h>

// {4EBCB545-53A2-4329-B40D-085627D9E963}
#define TRAY_LINUX_WINDOWHIDER_CID                      \
  { 0x4ebcb545, 0x53a2, 0x4329,                         \
    { 0xb4, 0xd, 0x8, 0x56, 0x27, 0xd9, 0xe9, 0x63 }    \
  }

#define TRAY_LINUX_WINDOWHIDER_CONTRACTID \
  "@codefisher.org/minimizetotray/window-hider;1"

class trayLinuxWindowHider: public trayIWindowHider {
public:
	NS_DECL_ISUPPORTS
	NS_DECL_TRAYIWINDOWHIDER

	trayLinuxWindowHider();
	static GdkFilterReturn WindowFilter(GdkXEvent *xevent, GdkEvent *event,
			gpointer data);

private:
	~trayLinuxWindowHider();

protected:
	/* additional members */
	nsIDOMWindow* m_watchDomWindow;
	GdkWindow** m_windowList;
	GdkWindow* m_watchedWindow;
	int m_windowListCount;

	/* attributes from idl */
	PRBool m_suppressed;
};

#endif /* TRAYLINUXWINDOWHIDER_CPP_ */
