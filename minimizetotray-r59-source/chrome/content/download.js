// Download specific functions for MinimizeToTray

if (!('extensions' in window))
    window.extensions = new Array();

if (!('minimizetotrayplus' in window.extensions))
    window.extensions.minimizetotrayplus = 
    function(){
        // this is a stub only
        throw Components.results.NS_ERROR_ABORT;
    };

// this allows the window to restore when activated
window.extensions.minimizetotrayplus.m_allowActivete = true;

window.extensions.minimizetotrayplus.trayWindowClose = function() {
    window.extensions.minimizetotrayplus.restore();
    window.close();
}

window.extensions.minimizetotrayplus.displayIcon = function () {
    try {
        // a turbo window must be open, or there is no way of restoring
        if(this.m_prefs.getBoolPref(this.k_pref_prefix + 'only-one-icon')
            && window.extensions.minimizetotrayplus.turboLoaded()){
            return true;
        }
    } catch(e) {
        // better just show it then
        return true;
    }
    return false;
};

// if there are active downloads, minimize else close
window.extensions.minimizetotrayplus.minimizableOnClose = function () {
	var nsIDownloadManager = Components.interfaces.nsIDownloadManager
	var downloadManager = Cc["@mozilla.org/download-manager;1"]
	                            .getService(nsIDownloadManager);
	var downloads = downloadManager.activeDownloads;
	while(downloads.hasMoreElements()){
		var download = downloads.getNext();
		if(download.state == nsIDownloadManager.DOWNLOAD_DOWNLOADING 
				|| download.state == nsIDownloadManager.DOWNLOAD_NOTSTARTED
				|| download.state == nsIDownloadManager.DOWNLOAD_PAUSED
				|| download.state == nsIDownloadManager.DOWNLOAD_SCANNING) {
			return true;
		}
	}
	return false;
}


// signal that the app-specific parts have done loading
window.extensions.minimizetotrayplus.m_bLoadedApp = true;
