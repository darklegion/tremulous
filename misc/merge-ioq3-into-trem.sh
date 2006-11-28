#! /bin/sh

cd /tmp

# Make sure previous copies are gone
rm -rf quake3/

# Checkout a new copy of ioq3
svn co svn://svn.icculus.org/quake3/trunk/ quake3
cd quake3/

IOQ3REVISION=`svnversion .`
IOQ3VERSION=ioq3-r$IOQ3REVISION

rm -rf $IOQ3VERSION
svn export . $IOQ3VERSION
cd $IOQ3VERSION

# Remove READMEs and the like
rm -rf BUGS ChangeLog README COPYING.txt NOTTODO TODO id-readme.txt web/

# Remove Q3 ui stuff
rm -rf ui/

# Remove Windows/VC related stuff
rm -f `find -iname "*.bat"`
rm -f `find -iname "*.lnt"`
rm -f `find -iname "*.sln"`
rm -f `find -iname "*.vcproj"`
rm -f `find -iname "*.def"`
rm -f `find -iname "*.q3asm"`
rm -f `find -iname "*.mak"`
rm -f `find -iname "*.ico"`
rm -f `find -iname "*.bmp"`
rm -f code/tools/lcc/makefile.nt
rm -rf code/win32/msvc/

# Remove Linux docs
rm -rf code/unix/LinuxSupport/
rm -rf code/unix/setup/
rm -f code/unix/README.*
rm -f code/unix/quake3.*

# Remove game code
rm -rf code/cgame/ code/game/ code/ui/ code/q3_ui/

mv code src

cd ..
svn_load_dirs svn://svn.icculus.org/tremulous/ -t upstream/$IOQ3VERSION upstream/current $IOQ3VERSION/
