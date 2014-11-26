// Thunderbird specific functions for MinimizeToTray

if (!('extensions' in window))
	window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions))
	window.extensions.minimizetotrayplus = function() {
		// this is a stub only
		throw Components.results.NS_ERROR_ABORT;
	};

window.extensions.minimizetotrayplus.displayIcon = function() {
	return !this.m_isTurboMode;
};

window.extensions.minimizetotrayplus.trayClick = function(event) {
	if (event.button == 0) {
		var self = window.extensions.minimizetotrayplus;
		if (!self.restore()) {
            if(window.windowState == window.STATE_MINIMIZED) {
			    window.restore();
            } else {
            	if(self.m_windowHider.windowActive()) {
            		this.minimizeWindow();
            	} else {
            		window.focus();
            	}
            }
		}
	}
};

window.extensions.minimizetotrayplus.alwaysShow = function() {
	if (this.m_isTurboMode || this.turboWindowExists()
			|| this.m_prefs.getBoolPref(this.k_pref_prefix + 'tray-always-show') == false) {
		return;
	}
	this.m_windowIcon.setTooltip(this.m_applicationName);
	this.m_windowIcon.showIcon();
	this.m_isTurboMode = true;
};

window.extensions.minimizetotrayplus.trayTurboClose = function() {
	this.m_isTurboMode = false;
};

window.extensions.minimizetotrayplus.minimizableOnClose = function() {
	return true;
};

// signal that the app-specific parts have done loading
window.extensions.minimizetotrayplus.m_bLoadedApp = true;
