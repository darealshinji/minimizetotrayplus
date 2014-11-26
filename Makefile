MAKE ?= make

all:
	rm -rf build/extensions/minimizetotray
	cp -r src build/extensions/minimizetotray
	cd build && ./build.sh

clean:
	$(MAKE) -C build clobber
	rm -f minimizetotray.xpi build/minimizetotray.zip
	cd build && rm -rf compiled traytmp extensions/minimizetotray

distclean: clean
	cd build/dependencies && \
	rm -rf linux-x86_64 linux-i686 macosx-i686 windows-i686 && \
	rm -f *.tar.lzma *.tar.bz2 ../nightingale.config

