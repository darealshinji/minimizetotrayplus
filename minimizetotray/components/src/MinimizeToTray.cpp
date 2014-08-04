// MinimizeToTray.cpp : Defines the entry point for the DLL application.
//

#include "trayMenuItem.h"

// the generic windows code
#ifdef _WIN32

#include "windows/trayWin32WindowIcon.h" // include first, one of the defines may case a crash other wise
#include "windows/trayWin32WindowHider.h"
#include "windows.h"

BOOL APIENTRY DllMain( HMODULE hModule,
		DWORD ul_reason_for_call,
		LPVOID lpReserved) {
	return TRUE;
}

#endif

// the generic linux code
#ifdef __linux

#include "trayLinuxWindowIcon.h"
#include "trayLinuxWindowHider.h"

#endif

// if we are using a version of Mozilla less then 2.0 (ie. Firefox 3.*)
#ifndef MOZILLA_VERSION_20

#include "nsIGenericFactory.h"

// window component rego
#ifdef _WIN32

NS_GENERIC_FACTORY_CONSTRUCTOR(trayWin32WindowHider);
NS_GENERIC_FACTORY_CONSTRUCTOR(trayWin32WindowIcon);
NS_GENERIC_FACTORY_CONSTRUCTOR(trayMenuItem);

static nsModuleComponentInfo components[] =
{
	{
		"Minimize To Tray - Window Hider - Win32",
		TRAY_WIN32_WINDOWHIDER_CID,
		TRAY_WIN32_WINDOWHIDER_CONTRACTID,
		trayWin32WindowHiderConstructor
	},
	{
		"Minimize To Tray - Window Icon - Win32",
		TRAY_WIN32_WINDOWICON_CID,
		TRAY_WIN32_WINDOWICON_CONTRACTID,
		trayWin32WindowIconConstructor
	},
	{
		"Minimize To Tray - Menu Item - Win32",
		TRAY_MENUITEM_CID,
		TRAY_MENUITEM_CONTRACTID,
		trayMenuItemConstructor
	}
};

#endif

// linux component rego
#ifdef __linux

NS_GENERIC_FACTORY_CONSTRUCTOR(trayLinuxWindowHider)
NS_GENERIC_FACTORY_CONSTRUCTOR(trayLinuxWindowIcon)
NS_GENERIC_FACTORY_CONSTRUCTOR(trayMenuItem)

static nsModuleComponentInfo components[] =
{
	{
		"Minimize To Tray - Window Hider - Linux",
		TRAY_LINUX_WINDOWHIDER_CID,
		TRAY_LINUX_WINDOWHIDER_CONTRACTID,
		trayLinuxWindowHiderConstructor
	},
	{
		"Minimize To Tray - Window Icon - Linux",
		TRAY_LINUX_WINDOWICON_CID,
		TRAY_LINUX_WINDOWICON_CONTRACTID,
		trayLinuxWindowIconConstructor
	},
	{
		"Minimize To Tray - Menu Item - Linux",
		TRAY_MENUITEM_CID,
		TRAY_MENUITEM_CONTRACTID,
		trayMenuItemConstructor
	}
};

#endif

NS_IMPL_NSGETMODULE(minimizeToTrayModule, components)

// this must be a version 2.0+ of Mozilla (ie. Firefox 4.*)
#else

#include "mozilla/ModuleUtils.h"
#include "nsIClassInfoImpl.h"

// window component rego
#ifdef _WIN32

NS_GENERIC_FACTORY_CONSTRUCTOR(trayWin32WindowHider);
NS_GENERIC_FACTORY_CONSTRUCTOR(trayWin32WindowIcon);
NS_GENERIC_FACTORY_CONSTRUCTOR(trayMenuItem);

NS_DEFINE_NAMED_CID(TRAY_WIN32_WINDOWHIDER_CID);
NS_DEFINE_NAMED_CID(TRAY_WIN32_WINDOWICON_CID);
NS_DEFINE_NAMED_CID(TRAY_MENUITEM_CID);

static const mozilla::Module::CIDEntry kTrayCIDs[] = {
    { &kTRAY_WIN32_WINDOWHIDER_CID, false, NULL, trayWin32WindowHiderConstructor },
    { &kTRAY_WIN32_WINDOWICON_CID, false, NULL, trayWin32WindowIconConstructor },
    { &kTRAY_MENUITEM_CID, false, NULL, trayMenuItemConstructor },
    { NULL }
 };
 
static const mozilla::Module::ContractIDEntry kTrayContracts[] = {
    { TRAY_WIN32_WINDOWHIDER_CONTRACTID, &kTRAY_WIN32_WINDOWHIDER_CID },
    { TRAY_WIN32_WINDOWICON_CONTRACTID, &kTRAY_WIN32_WINDOWICON_CID },
    { TRAY_MENUITEM_CONTRACTID, &kTRAY_MENUITEM_CID },
    { NULL }
};

#endif

// linux component rego
#ifdef __linux

NS_GENERIC_FACTORY_CONSTRUCTOR(trayLinuxWindowHider)
NS_GENERIC_FACTORY_CONSTRUCTOR(trayLinuxWindowIcon)
NS_GENERIC_FACTORY_CONSTRUCTOR(trayMenuItem)

NS_DEFINE_NAMED_CID(TRAY_LINUX_WINDOWHIDER_CID);
NS_DEFINE_NAMED_CID(TRAY_LINUX_WINDOWICON_CID);
NS_DEFINE_NAMED_CID(TRAY_MENUITEM_CID);

static const mozilla::Module::CIDEntry kTrayCIDs[] = {
    { &kTRAY_LINUX_WINDOWHIDER_CID, false, NULL, trayLinuxWindowHiderConstructor },
    { &kTRAY_LINUX_WINDOWICON_CID, false, NULL, trayLinuxWindowIconConstructor },
    { &kTRAY_MENUITEM_CID, false, NULL, trayMenuItemConstructor },
    { NULL }
 };
 
static const mozilla::Module::ContractIDEntry kTrayContracts[] = {
    { TRAY_LINUX_WINDOWHIDER_CONTRACTID, &kTRAY_LINUX_WINDOWHIDER_CID },
    { TRAY_LINUX_WINDOWICON_CONTRACTID, &kTRAY_LINUX_WINDOWICON_CID },
    { TRAY_MENUITEM_CONTRACTID, &kTRAY_MENUITEM_CID },
    { NULL }
};

#endif

static const mozilla::Module::CategoryEntry kTrayCategories[] = {
    { NULL }
};

static const mozilla::Module kTrayModule = {
    mozilla::Module::kVersion,
    kTrayCIDs,
    kTrayContracts,
    kTrayCategories
};


NSMODULE_DEFN(minimizeToTrayModule) = &kTrayModule;
NS_IMPL_MOZILLA192_NSGETMODULE(&kTrayModule)

#endif 
