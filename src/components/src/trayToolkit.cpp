#include "trayToolkit.h"

// these two are copied from /mozilla/widget/src/windows/nsToolkit.cpp
int ConvertAtoW(LPCSTR aStrInA, int aBufferSize, LPWSTR aStrOutW) {
	return MultiByteToWideChar(CP_ACP, 0, aStrInA, -1, aStrOutW, aBufferSize);
}

int ConvertWtoA(LPCWSTR aStrInW, int aBufferSizeOut, LPSTR aStrOutA) {
	if ((!aStrInW) || (!aStrOutA) || (aBufferSizeOut <= 0))
		return 0;

	int numCharsConverted = WideCharToMultiByte(CP_ACP, 0, aStrInW, -1,
			aStrOutA, aBufferSizeOut, "?", NULL);

	if (!numCharsConverted) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			// Overflow, add missing null termination but return 0
			aStrOutA[aBufferSizeOut - 1] = '\0';
		} else {
			// Other error, clear string and return 0
			aStrOutA[0] = '\0';
		}
	} else if (numCharsConverted < aBufferSizeOut) {
		// Add 2nd null (really necessary?)
		aStrOutA[numCharsConverted] = '\0';
	}

	return numCharsConverted;
}

PRBool trayToolkit::m_isInitialized = PR_FALSE;
PRBool trayToolkit::m_isNT = PR_FALSE;

TT_GetWindowLongPtr trayToolkit::pGetWindowLongPtr = ::GetWindowLongPtrA;
TT_SetWindowLongPtr trayToolkit::pSetWindowLongPtr = ::SetWindowLongPtrA;
TT_GetWindowText trayToolkit::pGetWindowText = trayToolkit::GetWindowTextA;
TT_Shell_NotifyIcon trayToolkit::pShell_NotifyIcon =
		trayToolkit::Shell_NotifyIconA;

void trayToolkit::Init() {
	if (trayToolkit::m_isInitialized)
		return;
	trayToolkit::m_isInitialized = PR_TRUE;

	OSVERSIONINFOEX osversion;
	BOOL osVersionInfoEx;

	::ZeroMemory(&osversion, sizeof(OSVERSIONINFOEX));
	osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(osVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osversion))) {
		// if OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
		osversion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osversion)) {
			// maybe we are running on very old Windows OS. Assign FALSE.
		}
	}

	trayToolkit::m_isNT = (osversion.dwPlatformId == VER_PLATFORM_WIN32_NT);
	if (trayToolkit::m_isNT) {
		trayToolkit::pGetWindowLongPtr = GetWindowLongPtrW;
		trayToolkit::pSetWindowLongPtr = SetWindowLongPtrW;
		trayToolkit::pGetWindowText = GetWindowTextW;
		trayToolkit::pShell_NotifyIcon = Shell_NotifyIconW;
	}
}

int WINAPI trayToolkit::GetWindowTextA(HWND hWnd, LPWSTR lpString, int nMaxCount) {
	LPSTR buffer = new CHAR[nMaxCount + 1];
	buffer[nMaxCount] = 0;
	lpString[0] = 0;
	int rv = ::GetWindowTextA(hWnd, buffer, nMaxCount);
	if (rv) {
		// the call to ::GetWindowTextA succeeded
		rv = ConvertAtoW(buffer, nMaxCount, lpString);
	}
	delete[] buffer;
	return rv;
}

BOOL WINAPI trayToolkit::Shell_NotifyIconA(DWORD dwMessage,
		PNOTIFYICONDATAW lpdata) {
	NOTIFYICONDATAA data;
	int version = 0;

	// now copy the data over
#if (NTDDI_VERSION >= NTDDI_WINXP)
	data.guidItem = lpdata->guidItem;
#endif

#if (NTDDI_VERSION >= NTDDI_WIN2K)
	data.dwState = lpdata->dwState;
	data.dwStateMask = lpdata->dwStateMask;
	ConvertWtoA(lpdata->szInfoTitle, sizeof(data.szInfoTitle), data.szInfoTitle);
	data.dwInfoFlags = lpdata->dwInfoFlags;
#endif
    data.hWnd = lpdata->hWnd;
    data.uID = lpdata->uID;
    data.uFlags = lpdata->uFlags;
    data.uCallbackMessage = lpdata->uCallbackMessage;
    data.hIcon = lpdata->hIcon;
    ConvertWtoA(lpdata->szTip, sizeof(data.szTip), data.szTip);

	return ::Shell_NotifyIconA(dwMessage, &data);
}
