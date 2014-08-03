/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#include "trayWin32WindowHider.h"
#include "trayRoutine.h"
#include "trayToolkit.h"

#include "nsCOMPtr.h"
#include "nsMemory.h"
#include "nsStringAPI.h"

#ifndef MOZILLA_VERSION_20
#include "widget/nsIBaseWindow.h"
#else
#include "nsIBaseWindow.h"
#endif

const TCHAR* trayWin32WindowHider::S_PROPINST = TEXT(
		"_MOOK_TRAY_WINDOWHIDER_INST");
const TCHAR* trayWin32WindowHider::S_PROPPROC = TEXT(
		"_MOOK_TRAY_WINDOWHIDER_PROC");

NS_IMPL_ISUPPORTS1(trayWin32WindowHider, trayIWindowHider)

trayWin32WindowHider::trayWin32WindowHider() {
	/* member initializers and constructor code */
	this->m_hwndList = NULL;
	this->m_hwndListCount = 0;

	/* attributes from idl */
	this->m_suppressed = FALSE;

	/* member initializers and constructor code for the watcher */
	this->m_hwnd = NULL;
	this->m_watchDomWindow = NULL;
	this->m_OldProc = NULL;
	trayToolkit::Init();
}

trayWin32WindowHider::~trayWin32WindowHider() {
	/* destructor code */
	if (0 < this->m_hwndListCount) {
		this->Restore();
	}

	/* part of the watcher code */
	if (this->m_hwnd) {
		// do not unhook - in case some other thing also subclassed this window
		//trayToolkit::pSetWindowLongPtr(this->m_hwnd, GWLP_WNDPROC, (LONG_PTR)this->m_OldProc);
		::RemoveProp(this->m_hwnd, S_PROPINST);
		::SetProp(this->m_hwnd, S_PROPPROC, (HANDLE) this->m_OldProc);
		this->m_hwnd = NULL;
	}
}

/* void minimize (
 *   in PRUint32 aCount,
 *   [array, size_is (aCount)] in nsIBaseWindow aBaseWindows);
 */

NS_IMETHODIMP trayWin32WindowHider::Minimize(PRUint32 aCount,
		nsIDOMWindow **aDOMWindows) {
	nsresult rv;
	PRUint32 i;

	NS_ENSURE_ARG(aCount);
	NS_ENSURE_ARG_POINTER(aDOMWindows);
	NS_ENSURE_TRUE(0 == this->m_hwndListCount, NS_ERROR_ALREADY_INITIALIZED);

	this->m_hwndList = new HWND[aCount];
	if (!this->m_hwndList) {
		return NS_ERROR_OUT_OF_MEMORY;
	}
	this->m_hwndListCount = aCount;

	for (i = 0; i < aCount; ++i) {
		nsCOMPtr<nsIBaseWindow> baseWindow;
		rv = GetBaseWindow(aDOMWindows[i], getter_AddRefs(baseWindow));
		NS_ENSURE_SUCCESS(rv, rv);

		nativeWindow aNativeWindow;
		rv = baseWindow->GetParentNativeWindow(&aNativeWindow);
		NS_ENSURE_SUCCESS(rv, rv);

		this->m_hwndList[i] = reinterpret_cast<HWND> (aNativeWindow);
		NS_ENSURE_ARG_POINTER(this->m_hwndList[i]);
	}

	// everything worked, now hide the window
	for (i = 0; i < aCount; ++i) {
		//if(::IsIconic(this->m_hwndList[i])){  // hopefully we never have to do this
		//	//	::ShowWindow(this->m_hwndList[i], SW_HIDE);
		//	/*
		//	 * Convoluted and ugly, but if it works, and since it is supposed to never happen, what the heck, nice to solve this properbly though
		//	 */
		//	HWND hwnd = this->m_hwndList[i];
		//	WINDOWPLACEMENT pl;
		//	pl.length = sizeof(WINDOWPLACEMENT);
		//	::GetWindowPlacement(hwnd, &pl);
		//	pl.showCmd = SW_RESTORE;//SW_SHOWNORMAL;
		//	::SetWindowPlacement(hwnd, &pl);
		//	pl.showCmd = SW_HIDE;
		//	::SetWindowPlacement(hwnd, &pl);
		//	//LOGGER((PRUnichar*)L"WOOT NO CRASH!");
		//} else {
		//	//::AnimateWindow(this->m_hwndList[i], 200, AW_HIDE | AW_HOR_POSITIVE | AW_VER_POSITIVE);
		//	::ShowWindow(this->m_hwndList[i], SW_HIDE);
		//}
		::ShowWindow(this->m_hwndList[i], SW_HIDE);
	}
	return NS_OK;
}

NS_IMETHODIMP trayWin32WindowHider::Restore() {

	NS_ENSURE_TRUE(0 < this->m_hwndListCount, NS_ERROR_NOT_INITIALIZED);

	for (PRInt32 i = 0; i < this->m_hwndListCount; ++i) {
		::ShowWindow(this->m_hwndList[i], SW_SHOW);
		if (::IsIconic(this->m_hwndList[i])) { // does this ever return true?
			::ShowWindow(this->m_hwndList[i], SW_RESTORE);
		}
	}
	delete[] this->m_hwndList;
	this->m_hwndList = nsnull;
	this->m_hwndListCount = 0;

	return NS_OK;
}

/* attribute boolean suppressed; */
NS_IMETHODIMP trayWin32WindowHider::GetSuppressed(PRBool *aSuppressed) {
	*aSuppressed = this->m_suppressed;
	return NS_OK;
}
NS_IMETHODIMP trayWin32WindowHider::SetSuppressed(PRBool aSuppressed) {
	this->m_suppressed = aSuppressed;
	return NS_OK;
}
