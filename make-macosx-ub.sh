#!/bin/sh

DESTDIR=build/release-darwin-ub
BASEDIR=base

BIN_OBJ="
	build/release-darwin-ppc/tremulous.ppc
	build/release-darwin-x86/tremulous.x86
"
BASE_OBJ="
	build/release-darwin-ppc/$BASEDIR/cgameppc.dylib
	build/release-darwin-x86/$BASEDIR/cgamex86.dylib
	build/release-darwin-ppc/$BASEDIR/uippc.dylib
	build/release-darwin-x86/$BASEDIR/uix86.dylib
	build/release-darwin-ppc/$BASEDIR/gameppc.dylib
	build/release-darwin-x86/$BASEDIR/gamex86.dylib
"
if [ ! -f Makefile ]; then
	echo "This script must be run from the ioquake3 build directory";
fi

if [ ! -d /Developer/SDKs/MacOSX10.2.8.sdk ]; then
	echo "
/Developer/SDKs/MacOSX10.2.8.sdk/ is missing, this doesn't install by default
with newer XCode releases, but you should be able to fine the installer at
/Applications/Installers/Xcode Tools/Packages/"
	exit 1;
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk ]; then
	echo "
/Developer/SDKs/MacOSX10.4u.sdk/ is missing.  You must install XCode 2.2 or 
newer in order to build Universal Binaries"
	exit 1;
fi

(BUILD_MACOSX_UB=ppc make && BUILD_MACOSX_UB=x86 make) || exit 1;

if [ ! -d $DESTDIR ]; then 
	mkdir $DESTDIR || exit 1;
fi
if [ ! -d $DESTDIR/$BASEDIR ]; then
	mkdir $DESTDIR/$BASEDIR || exit 1;
fi

echo "Installing Universal Binaries in $DESTDIR"
lipo -create -o $DESTDIR/Tremulous $BIN_OBJ
cp $BASE_OBJ $DESTDIR/$BASEDIR/
cp src/libs/macosx/*.dylib $DESTDIR/

