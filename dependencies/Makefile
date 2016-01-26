
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

FETCH := wget
MD5 := md5sum -c --status
TAR := tar

ifeq (linux,$(PLATFORM))
ifeq (x86_64,$(ARCH))
DEPDIRN = linux-x86_64
else
DEPDIRN = linux-i686
endif
TARBALL = $(DEPDIRN)-1.12-20130316-release-final.tar.lzma
endif

ifeq (windows,$(PLATFORM))
DEPDIRN = windows-i686
TARBALL = $(DEPDIRN)-1.12-20130121-release.tar.lzma
endif

ifeq (macosx,$(PLATFORM))
DEPDIRN = macosx-i686
TARBALL = $(DEPDIRN)-1.12.1-20140419.tar.bz2
FETCH := fetch
MD5 := md5 -r "$(TARBALL)" | grep -q -f
endif


all: dependencies.stamp

clean:

distclean:
	rm -rf linux-x86_64 linux-i686 windows-i686 macosx-i686
	rm -f *.tar.lzma *.tar.bz2 dependencies.stamp

dependencies.stamp: $(DEPDIRN)
	touch $@

$(DEPDIRN): $(TARBALL)
	$(MD5) "$(TARBALL).md5" || exit 1
	$(TAR) xf $(TARBALL)

$(TARBALL):
	$(FETCH) https://bitbucket.org/nightingale-media-player/nightingale-deps/downloads/$(TARBALL)

