#!/bin/sh
APPBUNDLE=Tremulous.app
BINARY=Tremulous.ub
PKGINFO=APPLTREM
ICNS=src/unix/MacSupport/Tremulous.icns
DESTDIR=build/release-darwin-ub
BASEDIR=base
Q3_VERSION=`grep "\#define Q3_VERSION" src/qcommon/q_shared.h | \
	sed -e 's/.*".* \([^ ]*\)"/\1/'`;

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
	echo "This script must be run from the Tremulous build directory";
fi

if [ ! -d /Developer/SDKs/MacOSX10.2.8.sdk ]; then
	echo "
/Developer/SDKs/MacOSX10.2.8.sdk/ is missing, this doesn't install by default
with newer XCode releases, but the installers is included"
	exit 1;
fi

if [ ! -d /Developer/SDKs/MacOSX10.4u.sdk ]; then
	echo "
/Developer/SDKs/MacOSX10.4u.sdk/ is missing.  You must install XCode 2.2 or 
newer in order to build Universal Binaries"
	exit 1;
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
cp $BASE_OBJ $DESTDIR/$APPBUNDLE/Contents/MacOS/$BASEDIR/
cp src/libs/macosx/*.dylib $DESTDIR/$APPBUNDLE/Contents/MacOS/

