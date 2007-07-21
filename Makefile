#
# Tremulous Makefile
#
# Nov '98 by Zoid <zoid@idsoftware.com>
#
# Loki Hacking by Bernd Kreimeier
#  and a little more by Ryan C. Gordon.
#  and a little more by Rafael Barrero
#  and a little more by the ioq3 cr3w
#  and a little more by Tim Angus
#
# GNU Make required
#

COMPILE_PLATFORM=$(shell uname|sed -e s/_.*//|tr '[:upper:]' '[:lower:]')

ifeq ($(COMPILE_PLATFORM),darwin)
  # Apple does some things a little differently...
  COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/x86/)
else
  COMPILE_ARCH=$(shell uname -m | sed -e s/i.86/x86/)
endif

BUILD_CLIENT     =
BUILD_CLIENT_SMP =
BUILD_SERVER     =
BUILD_GAME_SO    =
BUILD_GAME_QVM   =

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################
-include Makefile.local

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifndef ARCH
ARCH=$(COMPILE_ARCH)
endif

ifeq ($(ARCH),powerpc)
  ARCH=ppc
endif
export ARCH

ifneq ($(PLATFORM),$(COMPILE_PLATFORM))
  CROSS_COMPILING=1
else
  CROSS_COMPILING=0

  ifneq ($(ARCH),$(COMPILE_ARCH))
    CROSS_COMPILING=1
  endif
endif
export CROSS_COMPILING

ifndef COPYDIR
COPYDIR="/usr/local/games/tremulous"
endif

ifndef MOUNT_DIR
MOUNT_DIR=src
endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef GENERATE_DEPENDENCIES
GENERATE_DEPENDENCIES=1
endif

ifndef USE_CCACHE
USE_CCACHE=0
endif
export USE_CCACHE

ifndef USE_SDL
USE_SDL=1
endif

ifndef USE_OPENAL
USE_OPENAL=1
endif

ifndef USE_OPENAL_DLOPEN
USE_OPENAL_DLOPEN=0
endif

ifndef USE_CURL
USE_CURL=1
endif

ifndef USE_CURL_DLOPEN
  ifeq ($(PLATFORM),mingw32)
    USE_CURL_DLOPEN=0
  else
    USE_CURL_DLOPEN=1
  endif
endif

ifndef USE_CODEC_VORBIS
USE_CODEC_VORBIS=0
endif

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=1
endif

ifndef BUILD_MASTER_SERVER
BUILD_MASTER_SERVER=0
endif

#############################################################################

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)
CDIR=$(MOUNT_DIR)/client
SDIR=$(MOUNT_DIR)/server
RDIR=$(MOUNT_DIR)/renderer
CMDIR=$(MOUNT_DIR)/qcommon
UDIR=$(MOUNT_DIR)/unix
W32DIR=$(MOUNT_DIR)/win32
GDIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
NDIR=$(MOUNT_DIR)/null
UIDIR=$(MOUNT_DIR)/ui
JPDIR=$(MOUNT_DIR)/jpeg-6
TOOLSDIR=$(MOUNT_DIR)/tools
SDLHDIR=$(MOUNT_DIR)/SDL12
LIBSDIR=$(MOUNT_DIR)/libs
MASTERDIR=$(MOUNT_DIR)/master

# extract version info
VERSION=$(shell grep "\#define VERSION_NUMBER" $(CMDIR)/q_shared.h | \
  sed -e 's/[^"]*"\(.*\)"/\1/')

USE_SVN=
ifeq ($(wildcard .svn),.svn)
  SVN_REV=$(shell LANG=C svnversion .)
  ifneq ($(SVN_REV),)
    SVN_VERSION=$(VERSION)_SVN$(SVN_REV)
    USE_SVN=1
  endif
endif
ifneq ($(USE_SVN),1)
    SVN_VERSION=$(VERSION)
endif


#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

## Defaults
LIB=lib

INSTALL=install
MKDIR=mkdir

ifeq ($(PLATFORM),linux)

  ifeq ($(ARCH),alpha)
    ARCH=axp
  else
  ifeq ($(ARCH),x86_64)
    LIB=lib64
  else
  ifeq ($(ARCH),ppc64)
    LIB=lib64
  else
  ifeq ($(ARCH),s390x)
    LIB=lib64
  endif
  endif
  endif
  endif

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1
    ifeq ($(USE_OPENAL_DLOPEN),1)
      BASE_CFLAGS += -DUSE_OPENAL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_CURL),1)
    BASE_CFLAGS += -DUSE_CURL=1
    ifeq ($(USE_CURL_DLOPEN),1)
      BASE_CFLAGS += -DUSE_CURL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
  endif

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_VIDEO=1 -DUSE_SDL_SOUND=1 $(shell sdl-config --cflags)
  else
    BASE_CFLAGS += -I/usr/X11R6/include
  endif

  OPTIMIZE = -O3 -ffast-math -funroll-loops -fomit-frame-pointer

  ifeq ($(ARCH),x86_64)
    OPTIMIZE = -O3 -fomit-frame-pointer -ffast-math -funroll-loops \
      -falign-loops=2 -falign-jumps=2 -falign-functions=2 \
      -fstrength-reduce
    # experimental x86_64 jit compiler! you need GNU as
    HAVE_VM_COMPILED = true
  else
  ifeq ($(ARCH),x86)
    OPTIMIZE = -O3 -march=i586 -fomit-frame-pointer -ffast-math \
      -funroll-loops -falign-loops=2 -falign-jumps=2 \
      -falign-functions=2 -fstrength-reduce
    HAVE_VM_COMPILED=true
  else
  ifeq ($(ARCH),ppc)
    BASE_CFLAGS += -maltivec
    HAVE_VM_COMPILED=false
  endif
  endif
  endif

  ifneq ($(HAVE_VM_COMPILED),true)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  DEBUG_CFLAGS = $(BASE_CFLAGS) -g -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LDFLAGS=-lpthread
  LDFLAGS=-ldl -lm

  ifeq ($(USE_SDL),1)
    CLIENT_LDFLAGS=$(shell sdl-config --libs)
  else
    CLIENT_LDFLAGS=-L/usr/X11R6/$(LIB) -lX11 -lXext -lXxf86dga -lXxf86vm
  endif

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += -lopenal
    endif
  endif
 
  ifeq ($(USE_CURL),1)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LDFLAGS += -lcurl
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
  endif

  ifeq ($(ARCH),x86)
    # linux32 make ...
    BASE_CFLAGS += -m32
    LDFLAGS+=-m32
  endif

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MAC OS X
#############################################################################

ifeq ($(PLATFORM),darwin)
  HAVE_VM_COMPILED=true
  BASE_CFLAGS=
  CLIENT_LDFLAGS=
  LDFLAGS=
  OPTIMIZE=
  ifeq ($(BUILD_MACOSX_UB),ppc)
    CC=gcc-3.3
    BASE_CFLAGS += -arch ppc -DSMP \
      -DMAC_OS_X_VERSION_MIN_REQUIRED=1020 -nostdinc \
      -F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks \
      -I/Developer/SDKs/MacOSX10.2.8.sdk/usr/include/gcc/darwin/3.3 \
      -isystem /Developer/SDKs/MacOSX10.2.8.sdk/usr/include
    # when using the 10.2 SDK we are not allowed the two-level namespace so
    # in order to get the OpenAL dlopen() stuff to work without major
    # modifications, the controversial -m linker flag must be used.  this
    # throws a ton of multiply defined errors which cannot be suppressed.
    LDFLAGS += -arch ppc \
      -L/Developer/SDKs/MacOSX10.2.8.sdk/usr/lib/gcc/darwin/3.3 \
      -F/Developer/SDKs/MacOSX10.2.8.sdk/System/Library/Frameworks \
      -Wl,-syslibroot,/Developer/SDKs/MacOSX10.2.8.sdk,-m
    ARCH=ppc

    # OS X 10.2 sdk lacks dlopen() so ded would need libSDL anyway
    BUILD_SERVER=0

    # because of a problem with linking on 10.2 this will generate multiply
    # defined symbol errors.  The errors can be turned into warnings with
    # the -m linker flag, but you can't shut up the warnings
    USE_OPENAL_DLOPEN=1
  else
  ifeq ($(BUILD_MACOSX_UB),x86)
    CC=gcc-4.0
    BASE_CFLAGS += -arch i386 -DSMP \
      -mmacosx-version-min=10.4 \
      -DMAC_OS_X_VERSION_MIN_REQUIRED=1040 -nostdinc \
      -F/Developer/SDKs/MacOSX10.4u.sdk/System/Library/Frameworks \
      -I/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin8/4.0.1/include \
      -isystem /Developer/SDKs/MacOSX10.4u.sdk/usr/include
    LDFLAGS = -arch i386 -mmacosx-version-min=10.4 \
      -L/Developer/SDKs/MacOSX10.4u.sdk/usr/lib/gcc/i686-apple-darwin8/4.0.1 \
      -F/Developer/SDKs/MacOSX10.4u.sdk/System/Library/Frameworks \
      -Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk
    ARCH=x86
    BUILD_SERVER=0
  else
    # for whatever reason using the headers in the MacOSX SDKs tend to throw
    # errors even though they are identical to the system ones which don't
    # therefore we shut up warning flags when running the universal build
    # script as much as possible.
    BASE_CFLAGS += -Wall -Wimplicit -Wstrict-prototypes
  endif
  endif

  ifeq ($(ARCH),ppc)
    OPTIMIZE += -faltivec -O3
  endif
  ifeq ($(ARCH),x86)
    OPTIMIZE += -march=prescott -mfpmath=sse
    # x86 vm will crash without -mstackrealign since MMX instructions will be
    # used no matter what and they corrupt the frame pointer in VM calls
    BASE_CFLAGS += -mstackrealign
  endif

  BASE_CFLAGS += -fno-strict-aliasing -DMACOS_X -fno-common -pipe

  # Always include debug symbols...you can strip the binary later...
  BASE_CFLAGS += -gfull

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += -framework OpenAL
    else
      BASE_CFLAGS += -DUSE_OPENAL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_CURL),1)
    BASE_CFLAGS += -DUSE_CURL=1
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LDFLAGS += -lcurl
    else
      BASE_CFLAGS += -DUSE_CURL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
    CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
  endif

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_VIDEO=1 -DUSE_SDL_SOUND=1 -D_THREAD_SAFE=1 \
      -I$(SDLHDIR)/include
    # We copy sdlmain before ranlib'ing it so that subversion doesn't think
    #  the file has been modified by each build.
    LIBSDLMAIN=$(B)/libSDLmain.a
    LIBSDLMAINSRC=$(LIBSDIR)/macosx/libSDLmain.a
    CLIENT_LDFLAGS += -framework Cocoa -framework IOKit -framework OpenGL \
      $(LIBSDIR)/macosx/libSDL-1.2.0.dylib
  else
    # !!! FIXME: frameworks: OpenGL, Carbon, etc...
    #CLIENT_LDFLAGS += -L/usr/X11R6/$(LIB) -lX11 -lXext -lXxf86dga -lXxf86vm
  endif

  OPTIMIZE += -ffast-math -falign-loops=16

  ifneq ($(HAVE_VM_COMPILED),true)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  DEBUG_CFLAGS = $(BASE_CFLAGS) -g -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=dylib
  SHLIBCFLAGS=-fPIC -fno-common
  SHLIBLDFLAGS=-dynamiclib $(LDFLAGS)

  NOTSHLIBCFLAGS=-mdynamic-no-pic

else # ifeq darwin


#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifeq ($(PLATFORM),mingw32)

ifndef WINDRES
WINDRES=windres
endif

  ARCH=x86

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1 -DUSE_OPENAL_DLOPEN=1
  endif

  ifeq ($(USE_CURL),1)
    BASE_CFLAGS += -DUSE_CURL=1
    ifneq ($(USE_CURL_DLOPEN),1)
      BASE_CFLAGS += -DCURL_STATICLIB
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
  endif

  OPTIMIZE = -O3 -march=i586 -fomit-frame-pointer -ffast-math -falign-loops=2 \
    -funroll-loops -falign-jumps=2 -falign-functions=2 -fstrength-reduce

  HAVE_VM_COMPILED = true

  DEBUG_CFLAGS=$(BASE_CFLAGS) -g -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=dll
  SHLIBCFLAGS=
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  BINEXT=.exe

  LDFLAGS= -mwindows -lwsock32 -lgdi32 -lwinmm -lole32
  CLIENT_LDFLAGS=

  ifeq ($(USE_CURL),1)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LDFLAGS += $(LIBSDIR)/win32/libcurl.a
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
  endif

  ifeq ($(ARCH),x86)
    # build 32bit
    BASE_CFLAGS += -m32
    LDFLAGS+=-m32
  endif

  BUILD_SERVER = 0
  BUILD_CLIENT_SMP = 0

else # ifeq mingw32

#############################################################################
# SETUP AND BUILD -- FREEBSD
#############################################################################

ifeq ($(PLATFORM),freebsd)

  ifneq (,$(findstring alpha,$(shell uname -m)))
    ARCH=axp
  else #default to x86
    ARCH=x86
  endif #alpha test


  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes \
                -I/usr/X11R6/include

  DEBUG_CFLAGS=$(BASE_CFLAGS) -g

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1
    ifeq ($(USE_OPENAL_DLOPEN),1)
      BASE_CFLAGS += -DUSE_OPENAL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    BASE_CFLAGS += -DUSE_CODEC_VORBIS=1
  endif

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += $(shell sdl11-config --cflags) -DUSE_SDL_VIDEO=1 -DUSE_SDL_SOUND=1
  endif

  ifeq ($(ARCH),axp)
    BASE_CFLAGS += -DNO_VM_COMPILED
    RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3 -ffast-math -funroll-loops \
      -fomit-frame-pointer -fexpensive-optimizations
  else
  ifeq ($(ARCH),x86)
    RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3 -mtune=pentiumpro \
      -march=pentium -fomit-frame-pointer -pipe -ffast-math \
      -falign-loops=2 -falign-jumps=2 -falign-functions=2 \
      -funroll-loops -fstrength-reduce
    HAVE_VM_COMPILED=true
  else
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif
  endif

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LDFLAGS=-lpthread
  # don't need -ldl (FreeBSD)
  LDFLAGS=-lm

  CLIENT_LDFLAGS =

  ifeq ($(USE_SDL),1)
    CLIENT_LDFLAGS += $(shell sdl11-config --libs)
  else
    CLIENT_LDFLAGS += -L/usr/X11R6/$(LIB) -lGL -lX11 -lXext -lXxf86dga -lXxf86vm
  endif

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += $(THREAD_LDFLAGS) -lopenal
    endif
  endif

  ifeq ($(USE_CODEC_VORBIS),1)
    CLIENT_LDFLAGS += -lvorbisfile -lvorbis -logg
  endif


else # ifeq freebsd

#############################################################################
# SETUP AND BUILD -- NETBSD
#############################################################################

ifeq ($(PLATFORM),netbsd)

  ifeq ($(shell uname -m),i386)
    ARCH=x86
  endif

  LDFLAGS=-lm
  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)
  THREAD_LDFLAGS=-lpthread

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes
  DEBUG_CFLAGS=$(BASE_CFLAGS) -g

  ifneq ($(ARCH),x86)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  BUILD_CLIENT = 0
  BUILD_GAME_QVM = 0

else # ifeq netbsd

#############################################################################
# SETUP AND BUILD -- IRIX
#############################################################################

ifeq ($(PLATFORM),irix)

  ARCH=mips  #default to MIPS

  BASE_CFLAGS=-Dstricmp=strcasecmp -Xcpluscomm -woff 1185 -mips3 \
    -nostdinc -I. -I$(ROOT)/usr/include -DNO_VM_COMPILED
  RELEASE_CFLAGS=$(BASE_CFLAGS) -O3
  DEBUG_CFLAGS=$(BASE_CFLAGS) -g

  SHLIBEXT=so
  SHLIBCFLAGS=
  SHLIBLDFLAGS=-shared

  LDFLAGS=-ldl -lm
  CLIENT_LDFLAGS=-L/usr/X11/$(LIB) -lGL -lX11 -lXext -lm

else # ifeq IRIX

#############################################################################
# SETUP AND BUILD -- SunOS
#############################################################################

ifeq ($(PLATFORM),sunos)

  CC=gcc
  INSTALL=ginstall
  MKDIR=gmkdir
  COPYDIR="/usr/local/share/games/tremulous"

  ifneq (,$(findstring i86pc,$(shell uname -m)))
    ARCH=x86
  else #default to sparc
    ARCH=sparc
  endif

  ifneq ($(ARCH),x86)
    ifneq ($(ARCH),sparc)
      $(error arch $(ARCH) is currently not supported)
    endif
  endif


  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_SOUND=1 $(shell sdl-config --cflags)
  else
    BASE_CFLAGS += -I/usr/openwin/include
  endif

  OPTIMIZE = -O3 -ffast-math -funroll-loops

  ifeq ($(ARCH),sparc)
    OPTIMIZE = -O3 -ffast-math -falign-loops=2 \
      -falign-jumps=2 -falign-functions=2 -fstrength-reduce \
      -mtune=ultrasparc -mv8plus -mno-faster-structs \
      -funroll-loops
  else
  ifeq ($(ARCH),x86)
    OPTIMIZE = -O3 -march=i586 -fomit-frame-pointer -ffast-math \
      -funroll-loops -falign-loops=2 -falign-jumps=2 \
      -falign-functions=2 -fstrength-reduce
    HAVE_VM_COMPILED=true
    BASE_CFLAGS += -m32
    LDFLAGS += -m32
    BASE_CFLAGS += -I/usr/X11/include/NVIDIA
  endif
  endif

  ifneq ($(HAVE_VM_COMPILED),true)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  DEBUG_CFLAGS = $(BASE_CFLAGS) -ggdb -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  THREAD_LDFLAGS=-lpthread
  LDFLAGS=-lsocket -lnsl -ldl -lm

  BOTCFLAGS=-O0

  ifeq ($(USE_SDL),1)
    CLIENT_LDFLAGS=$(shell sdl-config --libs) -L/usr/X11/lib -lGLU -lX11 -lXext
  else
    CLIENT_LDFLAGS=-L/usr/openwin/$(LIB) -L/usr/X11/lib -lGLU -lX11 -lXext
  endif

else # ifeq sunos

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################
  BASE_CFLAGS=-DNO_VM_COMPILED
  DEBUG_CFLAGS=$(BASE_CFLAGS) -g
  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared

endif #Linux
endif #darwin
endif #mingw32
endif #FreeBSD
endif #NetBSD
endif #IRIX
endif #SunOS

TARGETS =

ifneq ($(BUILD_SERVER),0)
  TARGETS += $(B)/tremded.$(ARCH)$(BINEXT)
endif

ifneq ($(BUILD_CLIENT),0)
  TARGETS += $(B)/tremulous.$(ARCH)$(BINEXT)
  ifneq ($(BUILD_CLIENT_SMP),0)
    TARGETS += $(B)/tremulous-smp.$(ARCH)$(BINEXT)
  endif
endif

ifneq ($(BUILD_GAME_SO),0)
  TARGETS += \
    $(B)/base/cgame$(ARCH).$(SHLIBEXT) \
    $(B)/base/game$(ARCH).$(SHLIBEXT) \
    $(B)/base/ui$(ARCH).$(SHLIBEXT)
endif

ifneq ($(BUILD_GAME_QVM),0)
  ifneq ($(CROSS_COMPILING),1)
    TARGETS += \
      $(B)/base/vm/cgame.qvm \
      $(B)/base/vm/game.qvm \
      $(B)/base/vm/ui.qvm
  endif
endif

ifeq ($(USE_CCACHE),1)
  CC := ccache $(CC)
endif

ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\"
endif

ifeq ($(USE_LOCAL_HEADERS),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS=1
endif

ifeq ($(GENERATE_DEPENDENCIES),1)
  BASE_CFLAGS += -MMD
endif

ifeq ($(USE_SVN),1)
  BASE_CFLAGS += -DSVN_VERSION=\\\"$(SVN_VERSION)\\\"
endif

define DO_CC       
@echo "CC $<"
@$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
endef

define DO_SMP_CC
@echo "SMP_CC $<"
@$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -DSMP -o $@ -c $<
endef

define DO_BOT_CC
@echo "BOT_CC $<"
@$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(BOTCFLAGS) -DBOTLIB -o $@ -c $<
endef

ifeq ($(GENERATE_DEPENDENCIES),1)
  DO_QVM_DEP=cat $(@:%.o=%.d) | sed -e 's/\.o/\.asm/g' >> $(@:%.o=%.d)
endif

define DO_SHLIB_CC
@echo "SHLIB_CC $<"
@$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
@$(DO_QVM_DEP)
endef

define DO_AS
@echo "AS $<"
@$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
endef

define DO_DED_CC
@echo "DED_CC $<"
@$(CC) $(NOTSHLIBCFLAGS) -DDEDICATED $(CFLAGS) -o $@ -c $<
endef

define DO_WINDRES
@echo "WINDRES $<"
@$(WINDRES) -i $< -o $@
endef


#############################################################################
# MAIN TARGETS
#############################################################################

default: release
all: debug release

debug:
	@$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS)"
ifeq ($(BUILD_MASTER_SERVER),1)
	$(MAKE) -C $(MASTERDIR) debug
endif

release:
	@$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS)"
ifeq ($(BUILD_MASTER_SERVER),1)
	$(MAKE) -C $(MASTERDIR) release
endif

# Create the build directories and tools, print out
# an informational message, then start building
targets: makedirs tools
	@echo ""
	@echo "Building Tremulous in $(B):"
	@echo "  CC: $(CC)"
	@echo ""
	@echo "  CFLAGS:"
	@for i in $(CFLAGS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@echo "  Output:"
	@for i in $(TARGETS); \
	do \
		echo "    $$i"; \
	done
	@echo ""
	@$(MAKE) $(TARGETS)

makedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
	@if [ ! -d $(B)/clientsmp ];then $(MKDIR) $(B)/clientsmp;fi
	@if [ ! -d $(B)/ded ];then $(MKDIR) $(B)/ded;fi
	@if [ ! -d $(B)/base ];then $(MKDIR) $(B)/base;fi
	@if [ ! -d $(B)/base/cgame ];then $(MKDIR) $(B)/base/cgame;fi
	@if [ ! -d $(B)/base/game ];then $(MKDIR) $(B)/base/game;fi
	@if [ ! -d $(B)/base/ui ];then $(MKDIR) $(B)/base/ui;fi
	@if [ ! -d $(B)/base/qcommon ];then $(MKDIR) $(B)/base/qcommon;fi
	@if [ ! -d $(B)/base/vm ];then $(MKDIR) $(B)/base/vm;fi

#############################################################################
# QVM BUILD TOOLS
#############################################################################

Q3LCC=$(TOOLSDIR)/q3lcc$(BINEXT)
Q3ASM=$(TOOLSDIR)/q3asm$(BINEXT)

ifeq ($(CROSS_COMPILING),1)
tools:
	@echo QVM tools not built when cross-compiling
else
tools:
	$(MAKE) -C $(TOOLSDIR)/lcc install
	$(MAKE) -C $(TOOLSDIR)/asm install
endif

define DO_Q3LCC
@echo "Q3LCC $<"
@$(Q3LCC) -o $@ $<
endef

#############################################################################
# CLIENT/SERVER
#############################################################################

Q3OBJ = \
  $(B)/client/cl_cgame.o \
  $(B)/client/cl_cin.o \
  $(B)/client/cl_console.o \
  $(B)/client/cl_input.o \
  $(B)/client/cl_keys.o \
  $(B)/client/cl_main.o \
  $(B)/client/cl_net_chan.o \
  $(B)/client/cl_parse.o \
  $(B)/client/cl_scrn.o \
  $(B)/client/cl_ui.o \
  $(B)/client/cl_avi.o \
  \
  $(B)/client/cm_load.o \
  $(B)/client/cm_patch.o \
  $(B)/client/cm_polylib.o \
  $(B)/client/cm_test.o \
  $(B)/client/cm_trace.o \
  \
  $(B)/client/cmd.o \
  $(B)/client/common.o \
  $(B)/client/cvar.o \
  $(B)/client/files.o \
  $(B)/client/md4.o \
  $(B)/client/md5.o \
  $(B)/client/msg.o \
  $(B)/client/net_chan.o \
  $(B)/client/huffman.o \
  $(B)/client/parse.o \
  \
  $(B)/client/snd_adpcm.o \
  $(B)/client/snd_dma.o \
  $(B)/client/snd_mem.o \
  $(B)/client/snd_mix.o \
  $(B)/client/snd_wavelet.o \
  \
  $(B)/client/snd_main.o \
  $(B)/client/snd_codec.o \
  $(B)/client/snd_codec_wav.o \
  $(B)/client/snd_codec_ogg.o \
  \
  $(B)/client/qal.o \
  $(B)/client/snd_openal.o \
  \
  $(B)/client/cl_curl.o \
  \
  $(B)/client/sv_ccmds.o \
  $(B)/client/sv_client.o \
  $(B)/client/sv_game.o \
  $(B)/client/sv_init.o \
  $(B)/client/sv_main.o \
  $(B)/client/sv_net_chan.o \
  $(B)/client/sv_snapshot.o \
  $(B)/client/sv_world.o \
  \
  $(B)/client/q_math.o \
  $(B)/client/q_shared.o \
  \
  $(B)/client/unzip.o \
  $(B)/client/vm.o \
  $(B)/client/vm_interpreted.o \
  \
  $(B)/client/jcapimin.o \
  $(B)/client/jchuff.o   \
  $(B)/client/jcinit.o \
  $(B)/client/jccoefct.o  \
  $(B)/client/jccolor.o \
  $(B)/client/jfdctflt.o \
  $(B)/client/jcdctmgr.o \
  $(B)/client/jcphuff.o \
  $(B)/client/jcmainct.o \
  $(B)/client/jcmarker.o \
  $(B)/client/jcmaster.o \
  $(B)/client/jcomapi.o \
  $(B)/client/jcparam.o \
  $(B)/client/jcprepct.o \
  $(B)/client/jcsample.o \
  $(B)/client/jdapimin.o \
  $(B)/client/jdapistd.o \
  $(B)/client/jdatasrc.o \
  $(B)/client/jdcoefct.o \
  $(B)/client/jdcolor.o \
  $(B)/client/jddctmgr.o \
  $(B)/client/jdhuff.o \
  $(B)/client/jdinput.o \
  $(B)/client/jdmainct.o \
  $(B)/client/jdmarker.o \
  $(B)/client/jdmaster.o \
  $(B)/client/jdpostct.o \
  $(B)/client/jdsample.o \
  $(B)/client/jdtrans.o \
  $(B)/client/jerror.o \
  $(B)/client/jidctflt.o \
  $(B)/client/jmemmgr.o \
  $(B)/client/jmemnobs.o \
  $(B)/client/jutils.o \
  \
  $(B)/client/tr_animation.o \
  $(B)/client/tr_backend.o \
  $(B)/client/tr_bsp.o \
  $(B)/client/tr_cmds.o \
  $(B)/client/tr_curve.o \
  $(B)/client/tr_flares.o \
  $(B)/client/tr_font.o \
  $(B)/client/tr_image.o \
  $(B)/client/tr_init.o \
  $(B)/client/tr_light.o \
  $(B)/client/tr_main.o \
  $(B)/client/tr_marks.o \
  $(B)/client/tr_mesh.o \
  $(B)/client/tr_model.o \
  $(B)/client/tr_noise.o \
  $(B)/client/tr_scene.o \
  $(B)/client/tr_shade.o \
  $(B)/client/tr_shade_calc.o \
  $(B)/client/tr_shader.o \
  $(B)/client/tr_shadows.o \
  $(B)/client/tr_sky.o \
  $(B)/client/tr_surface.o \
  $(B)/client/tr_world.o \

ifeq ($(ARCH),x86)
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/ftola.o \
    $(B)/client/snapvectora.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifeq ($(ARCH),x86)
    Q3OBJ += $(B)/client/vm_x86.o
  endif
  ifeq ($(ARCH),x86_64)
    Q3OBJ += $(B)/client/vm_x86_64.o
  endif
  ifeq ($(ARCH),ppc)
    Q3OBJ += $(B)/client/vm_ppc.o
  endif
endif

ifeq ($(PLATFORM),mingw32)
  Q3OBJ += \
    $(B)/client/win_gamma.o \
    $(B)/client/win_glimp.o \
    $(B)/client/win_input.o \
    $(B)/client/win_main.o \
    $(B)/client/win_net.o \
    $(B)/client/win_qgl.o \
    $(B)/client/win_shared.o \
    $(B)/client/win_snd.o \
    $(B)/client/win_syscon.o \
    $(B)/client/win_wndproc.o \
    $(B)/client/win_resource.o
else
  Q3OBJ += \
    $(B)/client/unix_main.o \
    $(B)/client/unix_net.o \
    $(B)/client/unix_shared.o \
    $(B)/client/linux_signals.o \
    $(B)/client/linux_qgl.o \
    $(B)/client/linux_snd.o \
    $(B)/client/sdl_snd.o

  ifeq ($(PLATFORM),linux)
    Q3OBJ += $(B)/client/linux_joystick.o
  endif

  ifeq ($(USE_SDL),1)
    ifneq ($(PLATFORM),darwin)
      BUILD_CLIENT_SMP = 0
    endif
  endif

  Q3POBJ = \
    $(B)/client/linux_glimp.o \
    $(B)/client/sdl_glimp.o

  Q3POBJ_SMP = \
    $(B)/clientsmp/linux_glimp.o \
    $(B)/clientsmp/sdl_glimp.o
endif

$(B)/tremulous.$(ARCH)$(BINEXT): $(Q3OBJ) $(Q3POBJ) $(LIBSDLMAIN)
	@echo "LD $@"
	@$(CC) -o $@ $(Q3OBJ) $(Q3POBJ) $(CLIENT_LDFLAGS) \
		$(LDFLAGS) $(LIBSDLMAIN)

$(B)/tremulous-smp.$(ARCH)$(BINEXT): $(Q3OBJ) $(Q3POBJ_SMP) $(LIBSDLMAIN)
	@echo "LD $@"
	@$(CC) -o $@ $(Q3OBJ) $(Q3POBJ_SMP) $(CLIENT_LDFLAGS) \
		$(THREAD_LDFLAGS) $(LDFLAGS) $(LIBSDLMAIN)

ifneq ($(strip $(LIBSDLMAIN)),)
ifneq ($(strip $(LIBSDLMAINSRC)),)
$(LIBSDLMAIN) : $(LIBSDLMAINSRC)
	cp $< $@
	ranlib $@
endif
endif



#############################################################################
# DEDICATED SERVER
#############################################################################

Q3DOBJ = \
  $(B)/ded/sv_client.o \
  $(B)/ded/sv_ccmds.o \
  $(B)/ded/sv_game.o \
  $(B)/ded/sv_init.o \
  $(B)/ded/sv_main.o \
  $(B)/ded/sv_net_chan.o \
  $(B)/ded/sv_snapshot.o \
  $(B)/ded/sv_world.o \
  \
  $(B)/ded/cm_load.o \
  $(B)/ded/cm_patch.o \
  $(B)/ded/cm_polylib.o \
  $(B)/ded/cm_test.o \
  $(B)/ded/cm_trace.o \
  $(B)/ded/cmd.o \
  $(B)/ded/common.o \
  $(B)/ded/cvar.o \
  $(B)/ded/files.o \
  $(B)/ded/md4.o \
  $(B)/ded/msg.o \
  $(B)/ded/net_chan.o \
  $(B)/ded/huffman.o \
  $(B)/ded/parse.o \
  \
  $(B)/ded/q_math.o \
  $(B)/ded/q_shared.o \
  \
  $(B)/ded/unzip.o \
  $(B)/ded/vm.o \
  $(B)/ded/vm_interpreted.o \
  \
  $(B)/ded/linux_signals.o \
  $(B)/ded/unix_main.o \
  $(B)/ded/unix_net.o \
  $(B)/ded/unix_shared.o \
  \
  $(B)/ded/null_client.o \
  $(B)/ded/null_input.o \
  $(B)/ded/null_snddma.o

ifeq ($(ARCH),x86)
  Q3DOBJ += \
      $(B)/ded/ftola.o \
      $(B)/ded/snapvectora.o \
      $(B)/ded/matha.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifeq ($(ARCH),x86)
    Q3DOBJ += $(B)/ded/vm_x86.o
  endif
  ifeq ($(ARCH),x86_64)
    Q3DOBJ += $(B)/ded/vm_x86_64.o
  endif
  ifeq ($(ARCH),ppc)
    Q3DOBJ += $(B)/ded/vm_ppc.o
  endif
endif

$(B)/tremded.$(ARCH)$(BINEXT): $(Q3DOBJ)
	@echo "LD $@"
	@$(CC) -o $@ $(Q3DOBJ) $(LDFLAGS)



#############################################################################
## TREMULOUS CGAME
#############################################################################

CGOBJ_ = \
  $(B)/base/cgame/cg_main.o \
  $(B)/base/game/bg_misc.o \
  $(B)/base/game/bg_pmove.o \
  $(B)/base/game/bg_slidemove.o \
  $(B)/base/cgame/cg_consolecmds.o \
  $(B)/base/cgame/cg_buildable.o \
  $(B)/base/cgame/cg_animation.o \
  $(B)/base/cgame/cg_animmapobj.o \
  $(B)/base/cgame/cg_draw.o \
  $(B)/base/cgame/cg_drawtools.o \
  $(B)/base/cgame/cg_ents.o \
  $(B)/base/cgame/cg_event.o \
  $(B)/base/cgame/cg_marks.o \
  $(B)/base/cgame/cg_players.o \
  $(B)/base/cgame/cg_playerstate.o \
  $(B)/base/cgame/cg_predict.o \
  $(B)/base/cgame/cg_servercmds.o \
  $(B)/base/cgame/cg_snapshot.o \
  $(B)/base/cgame/cg_view.o \
  $(B)/base/cgame/cg_weapons.o \
  $(B)/base/cgame/cg_mem.o \
  $(B)/base/cgame/cg_scanner.o \
  $(B)/base/cgame/cg_attachment.o \
  $(B)/base/cgame/cg_trails.o \
  $(B)/base/cgame/cg_particles.o \
  $(B)/base/cgame/cg_ptr.o \
  $(B)/base/cgame/cg_tutorial.o \
  $(B)/base/ui/ui_shared.o \
  \
  $(B)/base/qcommon/q_math.o \
  $(B)/base/qcommon/q_shared.o

CGOBJ = $(CGOBJ_) $(B)/base/cgame/cg_syscalls.o
CGVMOBJ = $(CGOBJ_:%.o=%.asm) $(B)/base/game/bg_lib.asm

$(B)/base/cgame$(ARCH).$(SHLIBEXT) : $(CGOBJ)
	@echo "LD $@"
	@$(CC) $(SHLIBLDFLAGS) -o $@ $(CGOBJ)

$(B)/base/vm/cgame.qvm: $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm
	@echo "Q3ASM $@"
	@$(Q3ASM) -o $@ $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm



#############################################################################
## TREMULOUS GAME
#############################################################################

GOBJ_ = \
  $(B)/base/game/g_main.o \
  $(B)/base/game/bg_misc.o \
  $(B)/base/game/bg_pmove.o \
  $(B)/base/game/bg_slidemove.o \
  $(B)/base/game/g_mem.o \
  $(B)/base/game/g_active.o \
  $(B)/base/game/g_client.o \
  $(B)/base/game/g_cmds.o \
  $(B)/base/game/g_combat.o \
  $(B)/base/game/g_physics.o \
  $(B)/base/game/g_buildable.o \
  $(B)/base/game/g_misc.o \
  $(B)/base/game/g_missile.o \
  $(B)/base/game/g_mover.o \
  $(B)/base/game/g_session.o \
  $(B)/base/game/g_spawn.o \
  $(B)/base/game/g_svcmds.o \
  $(B)/base/game/g_target.o \
  $(B)/base/game/g_team.o \
  $(B)/base/game/g_trigger.o \
  $(B)/base/game/g_utils.o \
  $(B)/base/game/g_maprotation.o \
  $(B)/base/game/g_ptr.o \
  $(B)/base/game/g_weapon.o \
  $(B)/base/game/g_admin.o \
  \
  $(B)/base/qcommon/q_math.o \
  $(B)/base/qcommon/q_shared.o

GOBJ = $(GOBJ_) $(B)/base/game/g_syscalls.o
GVMOBJ = $(GOBJ_:%.o=%.asm) $(B)/base/game/bg_lib.asm

$(B)/base/game$(ARCH).$(SHLIBEXT) : $(GOBJ)
	@echo "LD $@"
	@$(CC) $(SHLIBLDFLAGS) -o $@ $(GOBJ)

$(B)/base/vm/game.qvm: $(GVMOBJ) $(GDIR)/g_syscalls.asm
	@echo "Q3ASM $@"
	@$(Q3ASM) -o $@ $(GVMOBJ) $(GDIR)/g_syscalls.asm



#############################################################################
## TREMULOUS UI
#############################################################################

UIOBJ_ = \
  $(B)/base/ui/ui_main.o \
  $(B)/base/ui/ui_atoms.o \
  $(B)/base/ui/ui_players.o \
  $(B)/base/ui/ui_shared.o \
  $(B)/base/ui/ui_gameinfo.o \
  \
  $(B)/base/game/bg_misc.o \
  $(B)/base/qcommon/q_math.o \
  $(B)/base/qcommon/q_shared.o

UIOBJ = $(UIOBJ_) $(B)/base/ui/ui_syscalls.o
UIVMOBJ = $(UIOBJ_:%.o=%.asm) $(B)/base/game/bg_lib.asm

$(B)/base/ui$(ARCH).$(SHLIBEXT) : $(UIOBJ)
	@echo "LD $@"
	@$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(UIOBJ)

$(B)/base/vm/ui.qvm: $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm
	@echo "Q3ASM $@"
	@$(Q3ASM) -o $@ $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm



#############################################################################
## CLIENT/SERVER RULES
#############################################################################

$(B)/client/%.o: $(UDIR)/%.s
	$(DO_AS)

$(B)/client/%.o: $(CDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(CMDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(BLIBDIR)/%.c
	$(DO_BOT_CC)

$(B)/client/%.o: $(JPDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(RDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(UDIR)/%.c
	$(DO_CC)

$(B)/clientsmp/%.o: $(UDIR)/%.c
	$(DO_SMP_CC)

$(B)/client/%.o: $(W32DIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(W32DIR)/%.rc
	$(DO_WINDRES)


$(B)/ded/%.o: $(UDIR)/%.s
	$(DO_AS)

$(B)/ded/%.o: $(SDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(CMDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(BLIBDIR)/%.c
	$(DO_BOT_CC)

$(B)/ded/%.o: $(UDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(NDIR)/%.c
	$(DO_DED_CC)

# Extra dependencies to ensure the SVN version is incorporated
ifeq ($(USE_SVN),1)
  $(B)/client/cl_console.o : .svn/entries
  $(B)/client/common.o : .svn/entries
  $(B)/ded/common.o : .svn/entries
endif


#############################################################################
## GAME MODULE RULES
#############################################################################

$(B)/base/cgame/%.o: $(CGDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/base/cgame/%.asm: $(CGDIR)/%.c
	$(DO_Q3LCC)


$(B)/base/game/%.o: $(GDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/base/game/%.asm: $(GDIR)/%.c
	$(DO_Q3LCC)


$(B)/base/ui/%.o: $(UIDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/base/ui/%.asm: $(UIDIR)/%.c
	$(DO_Q3LCC)


$(B)/base/qcommon/%.o: $(CMDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/base/qcommon/%.asm: $(CMDIR)/%.c
	$(DO_Q3LCC)


#############################################################################
# MISC
#############################################################################

clean: clean-debug clean-release
	@$(MAKE) -C $(MASTERDIR) clean

clean2:
	@echo "CLEAN $(B)"
	@if [ -d $(B) ];then (find $(B) -name '*.d' -exec rm {} \;)fi
	@rm -f $(Q3OBJ) $(Q3POBJ) $(Q3POBJ_SMP) $(Q3DOBJ) \
		$(GOBJ) $(CGOBJ) $(UIOBJ) \
		$(GVMOBJ) $(CGVMOBJ) $(UIVMOBJ)
	@rm -f $(TARGETS)

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

toolsclean:
	@$(MAKE) -C $(TOOLSDIR)/asm clean uninstall
	@$(MAKE) -C $(TOOLSDIR)/lcc clean uninstall

distclean: clean toolsclean
	@rm -rf $(BUILD_DIR)

dist:
	rm -rf tremulous-$(SVN_VERSION)
	svn export . tremulous-$(SVN_VERSION)
	tar --owner=root --group=root --force-local -cjf tremulous-$(SVN_VERSION).tar.bz2 tremulous-$(SVN_VERSION)
	rm -rf tremulous-$(SVN_VERSION)

#############################################################################
# DEPENDENCIES
#############################################################################

D_FILES=$(shell find . -name '*.d')

ifneq ($(strip $(D_FILES)),)
  include $(D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release \
	debug default dist distclean makedirs release \
	targets tools toolsclean
