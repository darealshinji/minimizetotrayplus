if (!('extensions' in window))
	window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions))
	window.extensions.minimizetotrayplus = function() {
		// this is a stub only
		throw Components.results.NS_ERROR_ABORT;
	};

window.extensions.minimizetotrayplus.displayIcon = function() {
	try {
		// a turbo window must be open, or there is no way of restoring
		if (this.m_prefs.getBoolPref(this.k_pref_prefix + 'only-one-icon') == false
				|| window.extensions.minimizetotrayplus.turboLoaded() == false) {
			return true;
		}
	} catch (e) {
		// better just show it then
		return true;
	}
	return false;
};

window.extensions.minimizetotrayplus.trayTurboClose = function() {
	for(var i in  this.trayWindows()) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isTurboMode) {
			// |w| is a turbo-mode window
			w.extensions.minimizetotrayplus.restore();
			w.close();
		}
	}
};

// WA: Fullscreen minimize-button Overlay
window.extensions.minimizetotrayplus.browserFullScreenMinimizeOnClick = function(event) {
	if (event.button == 2) {
		if (event.ctrlKey) {
			window.extensions.minimizetotrayplus.minimizeAll();
		} else {
			window.extensions.minimizetotrayplus.minimizeWindow();
		}
	} else if (event.button == 1) {
		window.extensions.minimizetotrayplus.minimizeAll();
	}
};

window.extensions.minimizetotrayplus.browserFullScreenMinimizeOnCommand = function() {
	if (window.extensions.minimizetotrayplus.m_prefs
			.getBoolPref(window.extensions.minimizetotrayplus.k_pref_prefix + 'always')) {
		window.extensions.minimizetotrayplus.minimizeWindow();
	} else {
		window.minimize();
	}
};

window.extensions.minimizetotrayplus.minimizableOnClose = function() {
	// See if the window has a menubar. This is important because
	// if the window does not have a way to select File-Exit because the
	// menu
	// bar is missing, then there isn't an easy way to close that window
	// So, the workaround is to normally close windows that whose
	// menubars are hidden.
	return document.documentElement.getAttribute("chromehidden").indexOf(
			"menubar") == -1;
};

if ('BrowserTryToCloseWindow' in this) {
	eval("BrowserTryToCloseWindow ="+BrowserTryToCloseWindow.toString().replace("{", "{ if(!window.extensions.minimizetotrayplus.extendedClose()) return; "));
}

// signal that the app-specific parts have done loading
window.extensions.minimizetotrayplus.m_bLoadedApp = true;
