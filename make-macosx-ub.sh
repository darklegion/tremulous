#!/bin/sh
APPBUNDLE=Tremulous.app
BINARY=Tremulous.ub
DEDBIN=tremded.ub
PKGINFO=APPLTREM
ICNS=misc/Tremulous.icns
DESTDIR=build/release-darwin-ub
BASEDIR=base
SDKDIR=""
Q3_VERSION=`grep "\#define Q3_VERSION" src/qcommon/q_shared.h | \
	sed -e 's/.*".* \([^ ]*\)"/\1/'`;

BIN_OBJ="
	build/release-darwin-ppc/tremulous.ppc
	build/release-darwin-x86/tremulous.x86
"
BIN_DEDOBJ="
	build/release-darwin-ppc/ioq3ded.ppc
	build/release-darwin-i386/ioq3ded.i386
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
	echo "This script must be run from the Tremulous build directory";
fi

# this is kind of a hack to find out the latest SDK to use. I assume that newer SDKs apear later in this for loop,
# thus the last valid one is the one we want.

for availsdks in /Developer/SDKs/*
do
	if [ -d $availsdks ]
	then
		SDKDIR="$availsdks"
	fi
done

if [ -z $SDKDIR ]
then
	echo "MacOSX SDK is missing. Please install a recent version of the MacOSX SDK."
	exit 1;
else
	echo "Using $SDKDIR for compilation"
fi

(BUILD_MACOSX_UB=ppc make && BUILD_MACOSX_UB=x86 make) || exit 1;

echo "Creating .app bundle $DESTDIR/$APPBUNDLE"
if [ ! -d $DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR ]; then
	mkdir -p $DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR || exit 1;
fi
if [ ! -d $DESTDIR/$APPBUNDLE/Contents/MacOS/$MPACKDIR ]; then
	mkdir -p $DESTDIR/$APPBUNDLE/Contents/MacOS/$MPACKDIR || exit 1;
fi
if [ ! -d $DESTDIR/$APPBUNDLE/Contents/Resources ]; then
	mkdir -p $DESTDIR/$APPBUNDLE/Contents/Resources
fi
cp $ICNS $DESTDIR/$APPBUNDLE/Contents/Resources/Tremulous.icns || exit 1;
echo $PKGINFO > $DESTDIR/$APPBUNDLE/Contents/PkgInfo
echo "
	<?xml version=\"1.0\" encoding="UTF-8"?>
	<!DOCTYPE plist
		PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\"
		\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
	<plist version=\"1.0\">
	<dict>
		<key>CFBundleDevelopmentRegion</key>
		<string>English</string>
		<key>CFBundleExecutable</key>
		<string>$BINARY</string>
		<key>CFBundleGetInfoString</key>
		<string>$Q3_VERSION</string>
		<key>CFBundleIconFile</key>
		<string>Tremulous.icns</string>
		<key>CFBundleIdentifier</key>
		<string>net.tremulous</string>
		<key>CFBundleInfoDictionaryVersion</key>
		<string>6.0</string>
		<key>CFBundleName</key>
		<string>Tremulous</string>
		<key>CFBundlePackageType</key>
		<string>APPL</string>
		<key>CFBundleShortVersionString</key>
		<string>$Q3_VERSION</string>
		<key>CFBundleSignature</key>
		<string>$PKGINFO</string>
		<key>CFBundleVersion</key>
		<string>$Q3_VERSION</string>
		<key>NSExtensions</key>
		<dict/>
		<key>NSPrincipalClass</key>
		<string>NSApplication</string>
	</dict>
	</plist>
	" > $DESTDIR/$APPBUNDLE/Contents/Info.plist

lipo -create -o $DESTDIR/$APPBUNDLE/Contents/MacOS/$BINARY $BIN_OBJ
lipo -create -o $DESTDIR/$APPBUNDLE/Contents/MacOS/$DEDBIN $BIN_DEDOBJ
cp $BASE_OBJ $DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR/
cp src/libs/macosx/*.dylib $DESTDIR/$APPBUNDLE/Contents/MacOS/

