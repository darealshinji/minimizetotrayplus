if (!('extensions' in window))
	window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions))
	window.extensions.minimizetotrayplus = function() {
		// this is a stub only
		throw Components.results.NS_ERROR_ABORT;
	};

window.extensions.minimizetotrayplus.trayClick = function(event) {
	if (event.button == 0) {
		var self = window.extensions.minimizetotrayplus;
		if (self.m_isHidden) {
			self.restore();
		} else if (!self.restoreWindow()) {
			self.openNewWindow();
		}
	}
}

window.extensions.minimizetotrayplus.openNewWindow = function() {
	var wwatch = Cc["@mozilla.org/embedcomp/window-watcher;1"]
			.getService(Ci.nsIWindowWatcher);
	return wwatch.openWindow(null, document.location.href, "_blank",
			"chrome,dialog=no,all", null);

};

window.extensions.minimizetotrayplus.displayIcon = function() {
	if (window.extensions.minimizetotrayplus.m_allwaysShow) {
		return false;
	}
	try {
		// a turbo window must be open, or there is no way of restoring
		if (this.m_prefs.getBoolPref(this.k_pref_prefix + 'only-one-icon')
				&& window.extensions.minimizetotrayplus.turboWindowExists()) {
			return false;
		}
	} catch (e) {
		// better just show it then
		return true;
	}
	return true;
};

window.extensions.minimizetotrayplus.alwaysShow = function() {
	if (this.m_prefs.getBoolPref(this.k_pref_prefix + 'tray-always-show')) {
		this.turboMode(false);
	}	
};

window.extensions.minimizetotrayplus.trayTurboClose = function() {
	for(var i in  this.trayWindows()) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isTurboMode) {
			// |w| is a turbo-mode window
			w.extensions.minimizetotrayplus.m_isTurboMode = false;
		} else if (w.extensions.minimizetotrayplus.m_windowIcon) {
			w.extensions.minimizetotrayplus.m_windowIcon.showIcon();
		}
	}
};

window.extensions.minimizetotrayplus.trayWindowClose = function() {
	self = window.extensions.minimizetotrayplus;
	window.close();
	self.m_windowIcon.hideIcon();
	if(self.m_isTurboMode && self.m_prefs.getBoolPref(self.k_pref_prefix + 'only-one-icon')) {
		self.restoreAllWindows();
	}
};

window.extensions.minimizetotrayplus.minimizableOnClose = function() {
	return true;
}

// signal that the app-specific parts have done loading
window.extensions.minimizetotrayplus.m_bLoadedApp = true;