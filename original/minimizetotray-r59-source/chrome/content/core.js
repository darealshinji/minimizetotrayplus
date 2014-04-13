// core functions for MinimizeToTray

if (!('extensions' in window))
	window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions)) {
	window.extensions.minimizetotrayplus = function() {
		// this is a stub only
		throw Components.results.NS_ERROR_ABORT;
	};
}

var Ci = Components.interfaces;
var Cc = Components.classes;

window.extensions.minimizetotrayplus.setConstants = function() {
	// /// constants
	this.k_contractid_windowHider  = '@codefisher.org/minimizetotray/window-hider;1';
	this.k_contractid_windowIcon   = '@codefisher.org/minimizetotray/window-icon;1';
	this.k_contractid_trayMenuItem = '@codefisher.org/minimizetotray/menu-item;1';
	this.k_pref_prefix 			   = 'extensions.minimizetotray.';
	this.k_xul_prefix              = 'extensions.minimizetotrayplus.';
	
	// /// component handles
	this.m_windowWatcher = null;
	this.m_windowHider   = null;
	this.m_prefs         = null;
	
	this.m_stringBundleService = Cc["@mozilla.org/intl/stringbundle;1"].getService(Ci.nsIStringBundleService);
	this.m_applicationName = Cc["@mozilla.org/xre/app-info;1"].getService(Ci.nsIXULAppInfo).name;
	this.m_stringsCommon = this.m_stringBundleService.createBundle("chrome://minimizetotray/locale/common.properties");
	this.nsIExtensionManager = null;
	if('nsIExtensionManager' in Ci)
		this.nsIExtensionManager = Cc['@mozilla.org/extensions/manager;1'].getService(Ci.nsIExtensionManager);
	
	// /// global variables, but not really constants
	this.m_isTurboMode = false;
	this.m_isHidden = false;
	this.m_isLocked = false;
	this.m_isMinimizing = false;
};
window.extensions.minimizetotrayplus.setConstants();

// ///methods
window.extensions.minimizetotrayplus.init = function() {

	// because |this| is |window|, where the event fired,
	// not |window.extensions.minimizetotrayplus|
	var self = window.extensions.minimizetotrayplus;

	// check to be sure that both app-neutral and app-specific parts
	// of the script has been loaded
	if (!('m_bLoadedCore' in self) || (!self.m_bLoadedCore)
			|| !('m_bLoadedApp' in self) || (!self.m_bLoadedApp)) {
		window.setTimeout(self.init, 10);
		return;
	}

	// remove the event handler
	window.removeEventListener("load",
			window.extensions.minimizetotrayplus.init, true);

	// we get an extra fire because the remove listener hasn't fired
	// so we just ignore the inconsistency - pretend it never happened
	if (self.m_prefs) {
		return;
	}
	// throw Components.results.NS_ERROR_ALREADY_INITIALIZED;
	
	self.noBinaryComponentWarning();

	self.m_prefs = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch);
	self.m_windowHider = Cc[self.k_contractid_windowHider].createInstance(Ci.trayIWindowHider);
	self.m_windowHider.watch(window);
	
	self.m_windowIcon = Cc[self.k_contractid_windowIcon].createInstance(Ci.trayIWindowIcon);
	self.m_windowIcon.setup(window, self.m_applicationName, self.trayMenuCallback);
	
	if (!self.initTurboMode()) { // turbo did not load
		if (self.alwaysShow) {
			self.alwaysShow();
		}
	}

	document.addEventListener("TrayMinimize", function(event) {
		self.minimizeWindow();
		event.preventDefault();
	}, true);
	document.addEventListener("TrayMinimizeAll", function(event) {
		self.minimizeAll();
		event.preventDefault();
	}, true);
	document.addEventListener("TrayRestore", function(event) {
		self.restore(); 
	}, true);
	document.addEventListener("TrayClose", function(event) {
		if (self.minimizableOnClose()) {
			self.minimizeWindow();
			event.preventDefault();
		}
	}, true);
	document.addEventListener("TrayTurboClose", function(event) {
		if (self.m_isTurboMode) {
			self.minimizeWindow();
			event.preventDefault();
		}
	}, true);


	window.addEventListener("TrayClick", function(event) {
		if (event.button == 0) {
			if (!self.m_prefs
					.getBoolPref(self.k_pref_prefix + 'two-click-restore')) {
				self.trayClick(event);
			}
		} else if(event.button == 2) {
			//document.getElementById(self.k_xul_prefix + 'traypopup')
			//	.openPopupAtScreen(event.screenX, event.screenY, true);
			event.preventDefault();
		}
	}, true);

	window.addEventListener("TrayDblClick", self.trayClick, true);

	// this allows the window to paint, which will cause the window to activate.
	// And we need to suppress that to make the turbo mode work correctly
	// but once the window is loaded we must remove that otherwise the window
	// can't be restored at all
	setTimeout(function() {
		self.m_windowHider.suppressed = false;		
		/* finally load our page if needed */
		self.load_update_page();
	}, 1000);
};

window.extensions.minimizetotrayplus.trayMenuCallback = function() {
	var trayMenu = Cc["@mozilla.org/array;1"].createInstance(Ci.nsIMutableArray);
	window.extensions.minimizetotrayplus.trayMenuCreate(trayMenu);
	return trayMenu;
};

window.extensions.minimizetotrayplus.shutdown = function() {
	var self = window.extensions.minimizetotrayplus;
	if (self.m_windowIcon) {
		self.m_windowIcon.hideIcon();
	}
};

window.extensions.minimizetotrayplus.trayWindows = function() {
	var wm = Cc["@mozilla.org/appshell/window-mediator;1"].getService(Ci.nsIWindowMediator);
	var e = wm.getEnumerator(null);
	var windows = [];
	while (e.hasMoreElements()) {
		var w = e.getNext();
		if ('extensions' in w && 'minimizetotrayplus' in w.extensions) {
			windows[windows.length] = w;
		}
	}
	return windows;
};

window.extensions.minimizetotrayplus.trayRestorableWindows = function() {
	var windows = this.trayWindows();
	var restorable = [];
	for ( var i in windows) {
		var w = windows[i];
		if(!w.extensions.minimizetotrayplus.m_isTurboMode 
				&& w.extensions.minimizetotrayplus.m_isHidden) {
			restorable[restorable.length] = restorable;
		}
	}
	return restorable;
}

window.extensions.minimizetotrayplus.trayClick = function(event) {
	if (event.button == 0) {
		window.extensions.minimizetotrayplus.restore();
	}
};

window.extensions.minimizetotrayplus.turboLoaded = function() {
	// try and find a turbo window
	var windows = window.extensions.minimizetotrayplus.trayWindows();
	for ( var i in windows) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isTurboMode) {
			return true; // found one !
		}
	}
	return false;
};

window.extensions.minimizetotrayplus.minimizeWindow = function() {
	this.noBinaryComponentWarning();
	this.restore();
	this.m_isMinimizing = true;
	this.m_isHidden = [ window ];
	this.m_windowHider.minimize(this.m_isHidden.length, this.m_isHidden);
	this.m_windowIcon.setTooltip(this.tooltip(null));
	if (this.displayIcon()) {
		this.m_windowIcon.showIcon();
	}
	this.m_isMinimizing = false;
};

window.extensions.minimizetotrayplus.minimizeAll = function() {
	this.noBinaryComponentWarning();
	this.restore();
	this.m_isMinimizing = true;
	var baseWindows = new Array();
	var wm = Cc["@mozilla.org/appshell/window-mediator;1"].getService(Ci.nsIWindowMediator);
	var e = wm.getEnumerator(null);
	while (e.hasMoreElements()) {
		var w = e.getNext();
		if (!('extensions' in w))
			w.extensions = new Array();
		if (!('minimizetotrayplus' in w.extensions)) {
			w.extensions.minimizetotrayplus = new Object();
			w.extensions.minimizetotrayplus.m_isHidden = false;
		}
		// don't hide turbo mode hidden windows... :p
		if (!w.extensions.minimizetotrayplus.m_isHidden
				&& !w.extensions.minimizetotrayplus.m_isTurboMode) {
			w.minimize();
			baseWindows[baseWindows.length] = w;
			w.extensions.minimizetotrayplus.m_isHidden = true;
		}
	}
	this.m_isHidden = baseWindows;
	var title = null;
	if (baseWindows.length > 1) {
		title = this.m_stringsCommon.formatStringFromName(
				"trayTooltipMultiple", [ baseWindows.length ], 1);
	}
	this.m_windowHider.minimize(baseWindows.length, baseWindows);
	this.m_windowIcon.setTooltip(this.tooltip(title));
	if (this.displayIcon()) {
		this.m_windowIcon.showIcon();
	}
	this.m_isMinimizing = false;
};

window.extensions.minimizetotrayplus.tooltip = function(title) {
	// pick the tooltip, the user may want just the application name,
	// because Windows uses it to decide of the icon should
	// allways show or not. But that is not our default
	try {
		if (this.m_prefs
				.getBoolPref(this.k_pref_prefix + 'use-application-name')) {
			return this.m_applicationName;
		}
	} catch (e) {
	}
	if(title) {
		return title;
	}
	return "";
};

window.extensions.minimizetotrayplus.turboMode = function(minimize) {
	var self = window.extensions.minimizetotrayplus;
	self.noBinaryComponentWarning();

	// look for existing -turbo mode windows
	if(self.m_isTurboMode || self.turboWindowExists()) {
		return false;
	}
	
	self.m_isTurboMode = true;
	
	// make the -turbo mode window
	if(minimize == undefined || minimize == true) {
		self.m_isHidden = [ window ];
		self.m_windowHider.suppressed = true;
		self.m_windowHider.minimize(self.m_isHidden.length, self.m_isHidden);
	}
	self.m_windowIcon.setTooltip(self.m_applicationName);
	self.m_windowIcon.showIcon();

	self.m_turboLoaded = true;
	return true;
};

window.extensions.minimizetotrayplus.restore = function() {
	self = window.extensions.minimizetotrayplus;
	if (self.m_isHidden && !self.m_isMinimizing) {
		var windows = self.m_isHidden;
		for ( var i = 0; i < windows.length; ++i) {
			var w = windows[i];
			// w is a window to be restored
			if (w) {
				w.extensions.minimizetotrayplus.m_isHidden = false;
			}
		}
		self.m_windowHider.restore();
		if (self.m_windowIcon && !self.m_isTurboMode) {
			self.m_windowIcon.hideIcon();
		}
		return true;
	}
	return false;
};

window.extensions.minimizetotrayplus.trayWindowClose = function() {
	// if we restore, we get the window flash,
	// so just cleanly removing the icon will do
	window.close();
	window.extensions.minimizetotrayplus.m_windowIcon.hideIcon();
};

window.extensions.minimizetotrayplus.noBinaryComponentWarning = function() {
	if (!(this.k_contractid_windowIcon in Cc && this.k_contractid_windowHider in Cc)) {
		// can't find binary component
		var promptService = Cc['@mozilla.org/embedcomp/prompt-service;1'].getService(Ci.nsIPromptService);
		promptService.alert(window, "Minimize To Tray - Component Load Error",
				'Cannot find binary component');
		// prevent popping up two warnings
		this.m_windowWatcher = true;
		throw Components.results.NS_ERROR_NOT_AVAILABLE;
	}
};

window.extensions.minimizetotrayplus.initTurboMode = function() {
	var commandLine = Cc['@codefisher.org/minimizetotray/turbo-startup;1'].getService(Ci.trayICommandLine);
	// see if not yet running in turbo mode, and if this is the case, launch a
	// turbo window
	if (commandLine.isTurboMode()) {
		return this.turboMode();
	}
	return false;
};

window.extensions.minimizetotrayplus.restoreAllWindows = function() {
	var windows = window.extensions.minimizetotrayplus.trayWindows();
	for ( var i in windows) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isHidden) {
			// |w| is a hidden window, it is not a turbo window,
			// and it is the main window if it was from a group
			// so we can restore it
			w.extensions.minimizetotrayplus.restore();
		}
	}
};

window.extensions.minimizetotrayplus.restoreWindow = function() {
	var windows = window.extensions.minimizetotrayplus.trayWindows();
	for (var i in windows) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isHidden) {
			// |w| is a hidden window and not a turbo window
			w.extensions.minimizetotrayplus.restore();
			return true;
		}
	}
	return false;
};

window.extensions.minimizetotrayplus.turboWindowExists = function() {
	var windows = window.extensions.minimizetotrayplus.trayWindows();
	for (var i in windows) {
		var w = windows[i];
		if (w.extensions.minimizetotrayplus.m_isTurboMode)
			// a turbo mode window already exists
			return w;
	}
	return false;
};

window.extensions.minimizetotrayplus.populateTurboMenu = function(trayMenu) {
	var wm = Cc["@mozilla.org/appshell/window-mediator;1"].getService(Ci.nsIWindowMediator);
	var e = wm.getEnumerator(null);
	var xe = wm.getXULWindowEnumerator(null);
	while (e.hasMoreElements() && xe.hasMoreElements()) {
		var w = e.getNext();
		var xw = xe.getNext();
		if ('extensions' in w && 'minimizetotrayplus' in w.extensions
				&& w.extensions.minimizetotrayplus.m_isHidden) {
			// |w| is a hidden window, it is not a turbo,
			// and it is the main window if it was from a group
			var docShell = xw.QueryInterface(Ci.nsIXULWindow).docShell;
			var title = docShell.contentViewer.DOMDocument.title.replace(/_/g, "__");
			if(title.length > 30) {
				title = title.substring(0, 30) + "\u2026";
			}
			this.creatMenuItem(trayMenu, "item", title , null, w.extensions.minimizetotrayplus.restore);
		}
	}
	return true;
};

/* functions used for displaying the page about the extension being updated */
window.extensions.minimizetotrayplus.file_put_contents = function(file, data) {
	var foStream = Cc['@mozilla.org/network/file-output-stream;1'].createInstance(Ci.nsIFileOutputStream);
	foStream.init(file, 0x02 | 0x08 | 0x20, 0664, 0);
	foStream.write(data, data.length);
	foStream.close();
};

window.extensions.minimizetotrayplus.load_url = function(url) {
	try {
		var newPage = getBrowser().addTab(url);
		getBrowser().selectedTab = newPage;
	} catch (e) {
		var uri = Cc['@mozilla.org/network/io-service;1'].getService(Ci.nsIIOService).newURI(url, null, null);
		var protocolSvc = Cc['@mozilla.org/uriloader/external-protocol-service;1'].getService(Ci.nsIExternalProtocolService);
		protocolSvc.loadUrl(uri);
	}
};

window.extensions.minimizetotrayplus.load_update_page = function() {
	var ext_id = "{de1b245c-de57-11da-ba2d-0050c2490048}";
	var version = null;
	if(this.nsIExtensionManager) {
		version = this.nsIExtensionManager.getItemForID(ext_id).version;
	} else {
		Components.utils.import("resource://gre/modules/AddonManager.jsm");
		AddonManager.getAddonByID(ext_id, function(addon) { 
			version = addon.version; 
		});
	}
	var url = "http://codefisher.org/minimizetotray/version/" + version;

	var extensionFlagFile = Cc["@mozilla.org/file/directory_service;1"].getService(Ci.nsIProperties).get("ProfD", Ci.nsIFile);
	extensionFlagFile.append("extensions");
	extensionFlagFile.append(ext_id);
	extensionFlagFile.append("installed");

	if (this.m_prefs.getCharPref(this.k_pref_prefix + "current-version") != version
			&& !extensionFlagFile.exists()) {
		this.m_prefs.setCharPref(this.k_pref_prefix + "current-version", version);
		this.file_put_contents(extensionFlagFile, version);
		this.load_url(url);
	}
};

window.extensions.minimizetotrayplus.creatMenuItem = function(trayMenu, type, label, access_key, func) {
	var os = Components.classes["@mozilla.org/xre/app-info;1"]
			.getService(Components.interfaces.nsIXULRuntime).OS;
	var menuItem = Cc[this.k_contractid_trayMenuItem].createInstance(Ci.trayIMenuItem);	
	var menuType = {
			"item": Ci.trayIMenuItem.MENU_ITEM,
			"separator": Ci.trayIMenuItem.SEPARATOR,
			"menu": Ci.trayIMenuItem.MENU
	}
	if(label != undefined) {
		if(access_key != null) {
			var regexp = new RegExp("(" + access_key + ")");
			if(!label.match(regexp)) {
				var regexp = new RegExp("(" + access_key + ")", "i");
			}
			if(os == "WINNT") {
				label = label.replace(regexp, "&$1");
			} else {
				label = label.replace(regexp, "_$1");
			}
		}
	} else {
		label = null;
		func = null;
	}
	menuItem.setup(menuType[type], label, func);
	trayMenu.appendElement(menuItem, false);
};

window.addEventListener("load", function() {
	window.extensions.minimizetotrayplus.init();
}, true);

window.addEventListener("close", function() {
	window.extensions.minimizetotrayplus.shutdown();
}, true);


window.extensions.minimizetotrayplus.extendedClose = function() {
	try { 
		var minimizetotray = window.extensions.minimizetotrayplus; 
		if (minimizetotray.m_prefs.getBoolPref(minimizetotray.k_pref_prefix + 'minimize-on-close-extended')) { 
			minimizetotray.minimizeWindow(); 
			return false; 
		} 
	} catch (ex) {
	}
	return true;
};

if ('closeWindow' in this) {
	// modify the closeWindow function so that if desired it will minimize
	// the window instead
	// the use of eval as always is evil/bad but TMP does it so we have too
	eval("closeWindow ="+closeWindow.toString().replace("{", "{ if(!window.extensions.minimizetotrayplus.extendedClose()) return false; "));
}

// signal that the core has loaded
window.extensions.minimizetotrayplus.m_bLoadedCore = true;
