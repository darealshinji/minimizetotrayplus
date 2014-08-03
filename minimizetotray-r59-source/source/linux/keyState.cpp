#include "keyState.h"
#include <X11/Xlib.h>
#define XK_MISCELLANY TRUE
#include <X11/keysymdef.h>

// http://www.ultimatepp.org/forum/index.php?t=msg&goto=23243&

bool keyIsPressed(accelKeysType key) {
	Display *display = XOpenDisplay(NULL);
	bool result = false;
	if (display != NULL) {
		char key_map_stat[32];
		XQueryKeymap(display, key_map_stat);
		if (key == SHIFT_KEY) {
			KeyCode shift_l = XKeysymToKeycode(display, XK_Shift_L);
			KeyCode shift_r = XKeysymToKeycode(display, XK_Shift_R);

			result = (((key_map_stat[shift_l >> 3] >> (shift_l & 7)) & 1)
					|| ((key_map_stat[shift_r >> 3] >> (shift_r & 7)) & 1));
		}

		if (key == CTRL_KEY) {
			KeyCode control_l = XKeysymToKeycode(display, XK_Control_L);
			KeyCode control_r = XKeysymToKeycode(display, XK_Control_R);

			result = (((key_map_stat[control_l >> 3] >> (control_l & 7)) & 1)
					|| ((key_map_stat[control_r >> 3] >> (control_r & 7)) & 1));
		}

		if (key == ALT_KEY) {
			KeyCode alt_l = XKeysymToKeycode(display, XK_Alt_L);
			KeyCode alt_r = XKeysymToKeycode(display, XK_Alt_R);

			result = (((key_map_stat[alt_l >> 3] >> (alt_l & 7)) & 1)
					|| ((key_map_stat[alt_r >> 3] >> (alt_r & 7)) & 1));
		}
		XCloseDisplay(display);
	}
	return result;
}
