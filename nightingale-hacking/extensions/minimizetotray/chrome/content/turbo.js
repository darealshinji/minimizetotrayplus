// Firefox specific functions for MinimizeToTray

if (!('extensions' in window))
	window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions))
	window.extensions.minimizetotrayplus = function() {
		// this is a stub only
		throw Components.results.NS_ERROR_ABORT;
	};

// since this is not in the main window, these will not be defined
var Ci = Components.interfaces;
var Cc = Components.classes;
	
window.extensions.minimizetotrayplus.m_isLocked = true;

window.extensions.minimizetotrayplus.trayClick = function(event) {
	if (event.button == 0) {
		if (!window.extensions.minimizetotrayplus.restoreWindow()) {
			OpenBrowserWindow();
		}
	}
};

window.extensions.minimizetotrayplus.trayTurboClose = function() {
	var windows = this.trayWindows();
	for(var i in windows) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isTurboMode) {
			// |w| is a turbo-mode window
			w.close();
			w.extensions.minimizetotrayplus.m_windowIcon.hideIcon();
		} else if (w.extensions.minimizetotrayplus.m_isHidden) {
			w.extensions.minimizetotrayplus.m_windowIcon.showIcon();
		}
	}
};

window.extensions.minimizetotrayplus.initTurboMode = function() {
	var self = window.extensions.minimizetotrayplus;
	var commandLine = Cc['@codefisher.org/minimizetotray/turbo-startup;1'].getService(Ci.trayICommandLine);
	if (self.m_isTurboMode) {
		return true;
	}
	window.focus(); window.restore();
	self.m_isTurboMode = true;
	self.m_windowHider.suppressed = true;

	var baseWindows = [ window ];
	self.m_windowHider.minimize(baseWindows.length, baseWindows);
	
	self.m_windowIcon.setTooltip(self.m_applicationName);
	self.m_windowIcon.showIcon();
	return true;
};

//signal that the app-specific parts have done loading
window.extensions.minimizetotrayplus.m_bLoadedApp = true;