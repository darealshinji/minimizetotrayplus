#ifndef KEYSTATE_H_
#define KEYSTATE_H_

typedef enum accelKeys {
	SHIFT_KEY, CTRL_KEY, ALT_KEY
} accelKeysType;

bool keyIsPressed(accelKeysType key);

#endif /* KEYSTATE_H_ */
