#!/bin/sh

set -e

ifminimizetotray=$(grep -e 'minimizetotray' extensions/Makefile.in)
if [ $ifminimizetotray = "" ] ; then
   echo 'ifeq (linux,$(SB_PLATFORM))' >> extensions/Makefile.in
   echo '    DEFAULT_EXTENSIONS += minimizetotray $(NULL)' >> extensions/Makefile.in
   echo 'endif' >> extensions/Makefile.in
fi

rm -rf compiled/extensions/minimizetotray \
 compiled/xpi-stage/minimizetotray/ \
 traytmp minimizetotray*.xpi

make run_autoconf
make run_configure
make -C compiled/dependencies
make -C compiled/extensions/minimizetotray

unzip compiled/xpi-stage/minimizetotray/*.xpi -d traytmp
chmod a-x `find traytmp -name *.so`
strip `find traytmp -name *.so`

cd traytmp
zip -r ../minimizetotray ./*
cd ..
mv minimizetotray.zip minimizetotray.xpi
