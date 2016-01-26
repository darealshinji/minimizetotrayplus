
ARCH := $(shell uname -m)
UNAME_S := $(shell uname -s)
ifeq (Linux,$(UNAME_S))
PLATFORM := linux
endif
ifneq (,$(filter MSYS% CYGWIN% MINGW32%,$(UNAME_S)))
PLATFORM := windows
endif
ifeq (Darwin,$(UNAME_S))
PLATFORM := macosx
endif

CXX  := g++
MAKE := make

ifeq (linux,$(PLATFORM))
ifeq (x86_64,$(ARCH))
DEPDIRN = linux-x86_64
else
DEPDIRN = linux-i686
endif
endif
ifeq (windows,$(PLATFORM))
DEPDIRN = windows-i686
endif
ifeq (macosx,$(PLATFORM))
DEPDIRN = macosx-i686
endif
VENDOR := $(CURDIR)/dependencies/$(DEPDIRN)/mozilla-1.9.2/release
BUILDDIR := $(CURDIR)/build

include src/extension-config.mk
EXTENSION      := minimizetotray-$(EXTENSION_VER).xpi
EXTENSION_ARCH := $(DEPDIRN)

CXXFLAGS := -O3
LDFLAGS  := -s

ifeq (linux,$(PLATFORM))
EXEEXT   :=
DLLEXT   := .so
CXXFLAGS += -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2
CXXFLAGS += -fPIC -DPIC -pthread
CXXFLAGS += $(shell pkg-config --cflags gtk+-2.0)
LDFLAGS  += -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,defs -Wl,--as-needed
endif

DEFINES := -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" \
	-DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" \
	-DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" \
	-DNDEBUG=1 -DXP_UNIX=1 -D_REENTRANT=1

INCLUDES := -include "$(VENDOR)/include/mozilla-config.h" \
	-I$(BUILDDIR) \
	-I$(VENDOR)/include/xpcom_glue \
	-I$(VENDOR)/include/xpcom \
	-I$(VENDOR)/include/string \
	-I$(VENDOR)/include/gfx \
	-I$(VENDOR)/include/widget \
	-I$(VENDOR)/include/docshell \
	-I$(VENDOR)/include/appshell \
	-I$(VENDOR)/include/dom \
	-I$(VENDOR)/include/content \
	-I$(VENDOR)/include/layout \
	-I$(VENDOR)/idl \
	-I$(VENDOR)/include \
	-I$(VENDOR)/include/xpcom \
	-I$(VENDOR)/include/string

CXXFLAGS += -fpermissive -fomit-frame-pointer -fshort-wchar -fexceptions -fnon-call-exceptions \
	-funwind-tables -fasynchronous-unwind-tables -fno-rtti -fno-strict-aliasing \
	-pipe \
	-Wall -Wno-conversion -Wno-attributes -Wpointer-arith -Wcast-align -Wno-long-long \
	-Woverloaded-virtual -Wsynth -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor \
	$(DEFINES) \
	$(INCLUDES)

LIBS := -L$(VENDOR)/lib -lxpcomglue_s -lnspr4 -lxpcom -lxul
ifeq (linux,$(PLATFORM))
LIBS += -lX11 $(shell pkg-config --libs gtk+-2.0)
endif

XPIDL    := $(VENDOR)/bin/xpidl$(EXEEXT)
XPT_LINK := $(VENDOR)/bin/xpt_link$(EXEEXT)
PERL     := perl
SED      := sed
ZIP      := zip

PERL_PP_FLAGS = $(DEFINES) -DSB_UPDATE_CHANNEL="default" -DSB_ENABLE_DEVICE_DRIVERS=1

OBJS = MinimizeToTray.o \
	trayRoutine.o \
	trayMenuItem.o
ifeq (linux,$(PLATFORM))
OBJS += trayLinuxWindowHider.o \
	trayLinuxWindowIcon.o \
	keyState.o
endif
ifeq (windows,$(PLATFORM))
OBJS += trayToolkit.o \
	trayWin32WindowHider.o \
	trayWin32WindowIcon.o \
	trayWin32WindowWatch.o
endif
SRCS = $(OBJS:.o=.cpp)


all: $(EXTENSION)

clean:
	rm -rf $(BUILDDIR)
	rm -f $(EXTENSION)

distclean: clean
	$(MAKE) -C dependencies distclean
	rm -f dependencies/dependencies.stamp

$(EXTENSION): dependencies/dependencies.stamp install.rdf minimizetotray.xpt minimizetotray.jar minimizetotray$(DLLEXT)
	mkdir -p $(BUILDDIR)/stage/chrome
	mkdir -p $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -rf $(CURDIR)/src/defaults $(BUILDDIR)/stage
	cp -f $(BUILDDIR)/minimizetotray.jar $(BUILDDIR)/stage/chrome
	cp -f $(CURDIR)/src/components/components.manifest $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -f $(CURDIR)/src/components/loader.js $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -f $(CURDIR)/src/components/trayCommandLine.js $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -f $(BUILDDIR)/minimizetotray.so $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -f $(BUILDDIR)/minimizetotray.xpt $(BUILDDIR)/stage/platform/$(EXTENSION_ARCH)/components
	cp -f $(BUILDDIR)/chrome.manifest $(BUILDDIR)/stage
	cp -f $(BUILDDIR)/install.rdf $(BUILDDIR)/stage
	cp -f $(CURDIR)/src/LICENSE $(BUILDDIR)/stage
	cd $(BUILDDIR)/stage && $(ZIP) -r $(CURDIR)/$@ *

dependencies/dependencies.stamp:
	$(MAKE) -C dependencies PLATFORM=$(PLATFORM) ARCH=$(ARCH)

minimizetotray$(DLLEXT): $(OBJS)
	cd $(BUILDDIR) && $(CXX) -o $@ -shared $(LDFLAGS) $^ $(LIBS)

$(OBJS): $(SRCS)
$(SRCS): trayInterface.h

%.o: %.cpp
	$(CXX) -o $(BUILDDIR)/$@ -c $(CURDIR)/src/components/src/$< $(CXXFLAGS)

$(BUILDDIR):
	mkdir -p $@

minimizetotray.xpt: trayInterface.h
	$(XPIDL) -m typelib -I$(VENDOR)/idl -I$(CURDIR) -e $(BUILDDIR)/$@ \
		$(CURDIR)/src/components/public/trayInterface.idl

trayInterface.h: $(BUILDDIR)
	cd $(BUILDDIR) && \
		$(XPIDL) -m header -I$(VENDOR)/idl $(CURDIR)/src/components/public/trayInterface.idl

minimizetotray.jar: jar.mn
	$(PERL) $(VENDOR)/scripts/make-jars.pl \
		-s $(CURDIR)/src/chrome -t $(CURDIR)/src \
		-p $(VENDOR)/scripts/preprocessor.pl \
		-j $(BUILDDIR) -d $(BUILDDIR) \
		-z $(ZIP) -v -e \
		-- $(PERL_PP_FLAGS) < $(BUILDDIR)/$<
	mv -f chrome.manifest $(BUILDDIR)

jar.mn: $(BUILDDIR)
	$(PERL) $(VENDOR)/scripts/preprocessor.pl $(PERL_PP_FLAGS) -- \
		$(CURDIR)/src/chrome/$@.in | \
		$(PERL) $(CURDIR)/expand-jar-mn.pl $(CURDIR)/src/chrome > $(BUILDDIR)/$@

install.rdf: $(BUILDDIR)
	$(SED) -e '/^#/d' src/install.rdf.in > $(BUILDDIR)/$@
	$(foreach VAR, EXTENSION_UUID EXTENSION_VER EXTENSION_ARCH EXTENSION_MIN_VER EXTENSION_MAX_VER,\
	  $(SED) -i 's|@$(VAR)@|$($(VAR))|' $(BUILDDIR)/$@; )

