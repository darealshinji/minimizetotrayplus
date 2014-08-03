#ifndef TRAYMENUITEM_H_
#define TRAYMENUITEM_H_

#include "trayInterface.h"
#include "nsEmbedString.h"
#include "nsCOMPtr.h"

// {204a88d4-8edc-11df-9b95-00247e68d2ed}
#define TRAY_MENUITEM_CID                      \
  { 0x204a88d4, 0x8edc, 0x11df,                         \
	{ 0x9b, 0x95, 0x00, 0x24, 0x7e, 0x68, 0xd2, 0xed }    \
  }

#define TRAY_MENUITEM_CONTRACTID \
	"@codefisher.org/minimizetotray/menu-item;1"

class trayMenuItem: public trayIMenuItem {
public:
	NS_DECL_ISUPPORTS
	NS_DECL_TRAYIMENUITEM

private:
	~trayMenuItem();

protected:
	/* additional members, the value, for public accss in C++ */
	PRUint32 m_menu_type;
    nsCOMPtr<trayICallback> m_callback;
    nsString m_label;
};

#endif /* TRAYMENUITEM_H_ */
