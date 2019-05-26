#
# Tremulous Makefile
#
# GNU Make required
#
COMPILE_PLATFORM=$(shell uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
COMPILE_ARCH=$(shell uname -m | sed -e 's/i.86/x86/' | sed -e 's/^arm.*/arm/')

ifeq ($(COMPILE_PLATFORM),sunos)
  # Solaris uname and GNU uname differ
  COMPILE_ARCH=$(shell uname -p | sed -e 's/i.86/x86/')
endif

ifeq ($(COMPILE_PLATFORM),linux)
  ifeq ($(COMPILE_ARCH),arm)
    # Get full arch name
  COMPILE_ARCH=$(shell file /bin/true | sed -e 's/^.*ld-linux-\(arm.*\)\.so.*/\1/')
  endif
endif

ifndef BUILD_STANDALONE
  BUILD_STANDALONE =
endif
ifndef BUILD_CLIENT
  BUILD_CLIENT     =
endif
ifndef BUILD_SERVER
  BUILD_SERVER     =
endif
ifndef BUILD_GRANGER
  BUILD_GRANGER    =
endif
ifndef BUILD_GAME_SO
  BUILD_GAME_SO    =
endif
ifndef BUILD_GAME_QVM
  BUILD_GAME_QVM   =
endif
ifndef BUILD_GAME_QVM_11
  BUILD_GAME_QVM_11 =
endif
ifndef BUILD_RENDERER_OPENGL2
  BUILD_RENDERER_OPENGL2=
endif

#############################################################################
#
# If you require a different configuration from the defaults below, create a
# new file named "Makefile.local" in the same directory as this file and define
# your parameters there. This allows you to change configuration without
# causing problems with keeping up to date with the repository.
#
#############################################################################
-include Makefile.local

ifeq ($(COMPILE_PLATFORM),cygwin)
  PLATFORM=mingw32
endif

ifndef PLATFORM
PLATFORM=$(COMPILE_PLATFORM)
endif
export PLATFORM

ifeq ($(PLATFORM),mingw32)
  MINGW=1
endif
ifeq ($(PLATFORM),mingw64)
  MINGW=1
endif

ifeq ($(COMPILE_ARCH),i86pc)
  COMPILE_ARCH=x86
endif

ifeq ($(COMPILE_ARCH),amd64)
  COMPILE_ARCH=x86_64
endif
ifeq ($(COMPILE_ARCH),x64)
  COMPILE_ARCH=x86_64
endif

ifeq ($(COMPILE_ARCH),powerpc)
  COMPILE_ARCH=ppc
endif
ifeq ($(COMPILE_ARCH),powerpc64)
  COMPILE_ARCH=ppc64
endif

ifeq ($(COMPILE_ARCH),axp)
  COMPILE_ARCH=alpha
endif

ifndef ARCH
ARCH=$(COMPILE_ARCH)
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

ifndef VERSION
VERSION=1.3.0
endif

ifndef CLIENTBIN
CLIENTBIN=tremulous
endif

ifndef SERVERBIN
SERVERBIN=tremded
endif

ifndef BASEGAME
BASEGAME=gpp
endif

BASEGAME_CFLAGS=-I../../${MOUNT_DIR}

ifndef COPYDIR
COPYDIR="/usr/local/games/tremulous"
endif

ifndef COPYBINDIR
COPYBINDIR=$(COPYDIR)
endif

ifndef MOUNT_DIR
MOUNT_DIR=src
endif

ifndef EXTERNAL_DIR
EXTERNAL_DIR=external
endif

ifndef ASSETS_DIR
ASSETS_DIR=assets
endif

ifndef BUILD_DIR
BUILD_DIR=build
endif

ifndef TEMPDIR
TEMPDIR=/tmp
endif

ifndef GENERATE_DEPENDENCIES
GENERATE_DEPENDENCIES=1
endif

ifndef USE_OPENAL
USE_OPENAL=1
endif

ifndef USE_OPENAL_DLOPEN
USE_OPENAL_DLOPEN=1
endif

ifndef USE_RESTCLIENT
USE_RESTCLIENT=1
USE_CURL=1
USE_CURL_DLOPEN=0
endif

ifndef USE_CURL
USE_CURL=1
endif

ifndef USE_CURL_DLOPEN
  ifdef MINGW
    USE_CURL_DLOPEN=0
  else
    USE_CURL_DLOPEN=1
  endif
endif

ifndef USE_CODEC_VORBIS
USE_CODEC_VORBIS=1
endif

ifndef USE_CODEC_OPUS
USE_CODEC_OPUS=1
endif

ifndef USE_MUMBLE
USE_MUMBLE=0
endif

ifndef USE_VOIP
USE_VOIP=0
endif

ifndef USE_FREETYPE
USE_FREETYPE=0
endif

ifndef USE_INTERNAL_LIBS
USE_INTERNAL_LIBS=1
endif

ifndef USE_INTERNAL_OGG
USE_INTERNAL_OGG=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_VORBIS
USE_INTERNAL_VORBIS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_OPUS
USE_INTERNAL_OPUS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_ZLIB
USE_INTERNAL_ZLIB=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_JPEG
USE_INTERNAL_JPEG=$(USE_INTERNAL_LIBS)
endif

ifndef USE_INTERNAL_LUA
USE_INTERNAL_LUA=$(USE_INTERNAL_LIBS)
endif

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=$(USE_INTERNAL_LIBS)
endif

ifndef USE_RENDERER_DLOPEN
USE_RENDERER_DLOPEN=1
endif

ifndef USE_YACC
USE_YACC=0
endif

ifndef DEBUG_CFLAGS
DEBUG_CFLAGS=-ggdb -O0
endif

ifndef BASE_CFLAGS
 BASE_CFLAGS=-fno-strict-aliasing
endif

#############################################################################

BD=$(BUILD_DIR)/debug-$(PLATFORM)-$(ARCH)
BR=$(BUILD_DIR)/release-$(PLATFORM)-$(ARCH)

CDIR=$(MOUNT_DIR)/client
SDIR=$(MOUNT_DIR)/server
RCOMMONDIR=$(MOUNT_DIR)/renderercommon
RGL1DIR=$(MOUNT_DIR)/renderergl1
RGL2DIR=$(MOUNT_DIR)/renderergl2
CMDIR=$(MOUNT_DIR)/qcommon
SDLDIR=$(MOUNT_DIR)/sdl
ASMDIR=$(MOUNT_DIR)/asm
SYSDIR=$(MOUNT_DIR)/sys
SCRIPTDIR=$(MOUNT_DIR)/script
GDIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
NDIR=$(MOUNT_DIR)/null
UIDIR=$(MOUNT_DIR)/ui
GRANGERDIR=$(MOUNT_DIR)/granger/src
JPDIR=$(EXTERNAL_DIR)/jpeg-8c
OGGDIR=$(EXTERNAL_DIR)/libogg-1.3.2
VORBISDIR=$(EXTERNAL_DIR)/libvorbis-1.3.5
OPUSDIR=$(EXTERNAL_DIR)/opus-1.1.4
OPUSFILEDIR=$(EXTERNAL_DIR)/opusfile-0.8
ZDIR=$(EXTERNAL_DIR)/zlib
LUADIR=$(EXTERNAL_DIR)/lua-5.3.3/src
RESTDIR=$(EXTERNAL_DIR)/restclient
NETTLEDIR=$(EXTERNAL_DIR)/nettle-3.3
SEMVERDIR=$(EXTERNAL_DIR)/semver
LUA_RAPIDJSONDIR=$(MOUNT_DIR)/script/rapidjson
Q3ASMDIR=$(MOUNT_DIR)/tools/asm
LBURGDIR=$(MOUNT_DIR)/tools/lcc/lburg
Q3CPPDIR=$(MOUNT_DIR)/tools/lcc/cpp
Q3LCCETCDIR=$(MOUNT_DIR)/tools/lcc/etc
Q3LCCSRCDIR=$(MOUNT_DIR)/tools/lcc/src
SDLHDIR=$(EXTERNAL_DIR)/SDL2
CURLHDIR=$(EXTERNAL_DIR)/libcurl-7.35.0
ALHDIR=$(EXTERNAL_DIR)/AL
LIBSDIR=$(EXTERNAL_DIR)/libs
TEMPDIR=/tmp

bin_path=$(shell which $(1) 2> /dev/null)

# We won't need this if we only build the server
ifneq ($(BUILD_CLIENT),0)
  # set PKG_CONFIG_PATH to influence this, e.g.
  # PKG_CONFIG_PATH=/opt/cross/i386-mingw32msvc/lib/pkgconfig
  ifneq ($(call bin_path, pkg-config),)
    CURL_CFLAGS ?= $(shell pkg-config --silence-errors --cflags libcurl)
    CURL_LIBS ?= $(shell pkg-config --silence-errors --libs libcurl)
    OPENAL_CFLAGS ?= $(shell pkg-config --silence-errors --cflags openal)
    OPENAL_LIBS ?= $(shell pkg-config --silence-errors --libs openal)
    SDL_CFLAGS ?= $(shell pkg-config --silence-errors --cflags sdl2|sed 's/-Dmain=SDL_main//')
    SDL_LIBS ?= $(shell pkg-config --silence-errors --libs sdl2)
  else
    # assume they're in the system default paths (no -I or -L needed)
    CURL_LIBS ?= -lcurl
    OPENAL_LIBS ?= -lopenal
  endif
  # Use sdl2-config if all else fails
  ifeq ($(SDL_CFLAGS),)
    ifneq ($(call bin_path, sdl2-config),)
      SDL_CFLAGS ?= $(shell sdl2-config --cflags)
      SDL_LIBS ?= $(shell sdl2-config --libs)
    endif
  endif
endif

# Add git version info
USE_GIT=
ifeq ($(wildcard .git),.git)
  GIT_REV=$(shell git describe --tag)
  ifneq ($(GIT_REV),)
    VERSION:=$(GIT_REV)
    USE_GIT=1
  endif
endif


#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

INSTALL=install
MKDIR=mkdir
EXTRA_FILES=
CLIENT_EXTRA_FILES=

ifneq (,$(findstring "$(PLATFORM)", "linux" "gnu_kfreebsd" "kfreebsd-gnu" "gnu"))
  BASE_CFLAGS += -DUSE_ICON
  CLIENT_CFLAGS += $(SDL_CFLAGS)

  OPTIMIZEVM = -O3
  OPTIMIZE = $(OPTIMIZEVM) -ffast-math

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -msse2
    HAVE_VM_COMPILED = true
  else
  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -msse2 -mfpmath=387+sse
    HAVE_VM_COMPILED=true
  else
  ifeq ($(ARCH),ppc)
    BASE_CFLAGS += -maltivec
    HAVE_VM_COMPILED=true
  endif
  ifeq ($(ARCH),ppc64)
    BASE_CFLAGS += -maltivec
    HAVE_VM_COMPILED=true
  endif
  ifeq ($(ARCH),sparc)
    OPTIMIZE += -mtune=ultrasparc3 -mv8plus
    OPTIMIZEVM += -mtune=ultrasparc3 -mv8plus
    HAVE_VM_COMPILED=true
  endif
  ifeq ($(ARCH),alpha)
    # According to http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=410555
    # -ffast-math will cause the client to die with SIGFPE on Alpha
    OPTIMIZE = $(OPTIMIZEVM)
  endif
  ifeq ($(ARCH),armhf)
    BASE_CFLAGS += -D__armhf__
  endif
  endif
  endif

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC -fvisibility=hidden
  SHLIBLDFLAGS=-shared
  #$(LDFLAGS)

  THREAD_LIBS=-lpthread
  LIBS=-ldl -lm
  GRANGER_LIBS=-lm -ldl

  CLIENT_LIBS=$(SDL_LIBS)
  RENDERER_LIBS = $(SDL_LIBS) -lGL

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  ifeq ($(USE_MUMBLE),1)
    CLIENT_LIBS += -lrt
  endif

  ifeq ($(ARCH),x86)
    # linux32 make ...
    BASE_CFLAGS += -m32
  else
  ifeq ($(ARCH),ppc64)
    BASE_CFLAGS += -m64
  endif
  endif
else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MAC OS X
#############################################################################

ifeq ($(PLATFORM),darwin)
  HAVE_VM_COMPILED=true
  LIBS = -framework Cocoa
  CLIENT_LIBS=
  RENDERER_LIBS=
  OPTIMIZEVM=
  #CXXFLAGS+=-stdlib=libc++

  # FIXME This is probably bad idea to comment this out 
  #BASE_CFLAGS += -mmacosx-version-min=10.7 -DMAC_OS_X_VERSION_MIN_REQUIRED=1070

  GRANGER_LIBS = -framework Cocoa -framework Security

  ifeq ($(USE_RESTCLIENT),1)
    CLIENT_LIBS += -framework Security
  endif

  ifeq ($(ARCH),ppc)
    BASE_CFLAGS += -arch ppc -faltivec
    OPTIMIZEVM += -O3
  endif
  ifeq ($(ARCH),ppc64)
    BASE_CFLAGS += -arch ppc64 -faltivec
  endif
  ifeq ($(ARCH),x86)
    OPTIMIZEVM += -mfpmath=387+sse
    # x86 vm will crash without -mstackrealign since MMX instructions will be
    # used no matter what and they corrupt the frame pointer in VM calls
    BASE_CFLAGS += -arch i386 -m32 -mstackrealign
  endif
  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM += -arch x86_64 -mfpmath=sse -msse2
  endif

  # When compiling on OSX for OSX, we're not cross compiling as far as the
  # Makefile is concerned, as target architecture is specified as a compiler
  # argument
  ifeq ($(COMPILE_PLATFORM),darwin)
    CROSS_COMPILING=0
  endif

  ifeq ($(CROSS_COMPILING),1)
    ifeq ($(ARCH),x86_64)
      CC=x86_64-apple-darwin13-cc
      RANLIB=x86_64-apple-darwin13-ranlib
    else
      ifeq ($(ARCH),x86)
        CC=i386-apple-darwin13-cc
        RANLIB=i386-apple-darwin13-ranlib
      else
        $(error Architecture $(ARCH) is not supported when cross compiling)
      endif
    endif
  endif

  BASE_CFLAGS += -fno-strict-aliasing -fno-common

  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += -framework OpenAL
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifneq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  BASE_CFLAGS += -D_THREAD_SAFE=1

  # FIXME: It is not possible to build using system SDL2 framework
  #  1. IF you try, this Makefile will still drop libSDL-2.0.0.dylib into the builddir
  #  2. Debugger warns that you have 2- which one will be used is undefined
  ifeq ($(USE_LOCAL_HEADERS),1)
    BASE_CFLAGS += -I$(SDLHDIR)/include -I$(CURLHDIR) -I$(ALHDIR)
  endif

  # We copy sdlmain before ranlib'ing it so that subversion doesn't think
  #  the file has been modified by each build.
  LIBSDLMAIN=$(B)/libSDL2main.a
  LIBSDLMAINSRC=$(LIBSDIR)/macosx/libSDL2main.a
  CLIENT_LIBS += -framework IOKit \
    $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib
  RENDERER_LIBS += -framework OpenGL $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib
  CLIENT_EXTRA_FILES += $(LIBSDIR)/macosx/libSDL2-2.0.0.dylib

  OPTIMIZE = $(OPTIMIZEVM) -ffast-math

  SHLIBEXT=dylib
  SHLIBCFLAGS=-fPIC -fno-common
  #SHLIBLDFLAGS=-dynamiclib $(LDFLAGS) -Wl,-U,_com_altivec
  SHLIBLDFLAGS=-dynamiclib -Wl,-U,_com_altivec
 
  NOTSHLIBCFLAGS=-mdynamic-no-pic

else # ifeq darwin


#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifdef MINGW

  ifeq ($(CROSS_COMPILING),1)
    # If CC is already set to something generic, we probably want to use
    # something more specific
    ifneq ($(findstring $(strip $(CC)),cc gcc),)
      CC=
    endif
    ifneq ($(findstring $(strip $(CXX)),c++ g++),)
      CXX=
    endif

    # We need to figure out the correct gcc and windres
    ifeq ($(ARCH),x86_64)
      MINGW_PREFIXES=x86_64-w64-mingw32
    endif
    ifeq ($(ARCH),x86)
      MINGW_PREFIXES=i686-w64-mingw32
    endif

    ifndef CC
      CC=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), $(call bin_path, $(MINGW_PREFIX)-gcc))))
    endif
    ifndef CXX
      CXX=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), $(call bin_path, $(MINGW_PREFIX)-g++))))
    endif

    ifndef WINDRES
      WINDRES=$(firstword $(strip $(foreach MINGW_PREFIX, $(MINGW_PREFIXES), $(call bin_path, $(MINGW_PREFIX)-windres))))
    endif
  else
    # Some MinGW installations define CC to cc, but don't actually provide cc,
    # so check that CC points to a real binary and use gcc if it doesn't
    ifeq ($(call bin_path, $(CC)),)
      CC=gcc
    endif

    ifeq ($(call bin_path, $(CXX)),)
      CXX=g++
    endif

    ifndef WINDRES
      WINDRES=windres
    endif
  endif

  ifeq ($(CC),)
    $(error Cannot find a suitable cross compiler for $(PLATFORM) CC)
  endif
  ifeq ($(CXX),)
    $(error Cannot find a suitable cross compiler for $(PLATFORM) CXX)
  endif

  CFLAGS += -static -static-libgcc -static-libstdc++
  CXXFLAGS += -static -static-libgcc -static-libstdc++
  LDFLAGS += -static -static-libgcc -static-libstdc++
  GRANGER_CFLAGS = -D_CRT_SECURE_NO_WARNINGS

  BASE_CFLAGS += -DUSE_ICON

  # In the absence of wspiapi.h, require Windows XP or later
  ifeq ($(shell test -e $(CMDIR)/wspiapi.h; echo $$?),1)
  	# FIXIT-L Update WINVER=_WIN32_WINNT_WIN7 (see https://msdn.microsoft.com/en-us/library/6sehtctf.aspx)
    BASE_CFLAGS += -DWINVER=0x501
  endif

  ifeq ($(USE_OPENAL),1)
    CLIENT_CFLAGS += $(OPENAL_CFLAGS)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += $(OPENAL_LDFLAGS)
    endif
    ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(ALHDIR) 
    endif
  endif

  ifeq ($(ARCH),x86_64)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -msse2
    HAVE_VM_COMPILED = true
    BASE_CFLAGS += -m64
  endif

  ifeq ($(ARCH),x86)
    OPTIMIZEVM = -O3
    OPTIMIZE = $(OPTIMIZEVM) -ffast-math -msse2 -mfpmath=387+sse
    HAVE_VM_COMPILED = true
    BASE_CFLAGS += -m32
  endif

  SHLIBEXT=dll
  SHLIBCFLAGS=
  #SHLIBLDFLAGS=-shared $(LDFLAGS)
  SHLIBLDFLAGS=-shared

  BINEXT=.exe

  ifeq ($(CROSS_COMPILING),0)
    TOOLS_BINEXT=.exe
  endif

  ifeq ($(COMPILE_PLATFORM),cygwin)
    TOOLS_BINEXT=.exe
    TOOLS_CC=$(CC)
  endif

  LIBS= -lws2_32 -lwinmm -lpsapi
  # clang 3.4 doesn't support this
  ifneq ("$(CC)", $(findstring "$(CC)", "clang" "clang++"))
    CLIENT_LDFLAGS += -mwindows
  endif
  CLIENT_LIBS = -lgdi32 -lole32
  RENDERER_LIBS = -lgdi32 -lole32 -lopengl32

  ifeq ($(USE_FREETYPE),1)
    FREETYPE_CFLAGS = -Ifreetype2
  endif

  ifeq ($(USE_CURL),1)
    ifneq ($(USE_CURL_DLOPEN),1)
      ifeq ($(USE_LOCAL_HEADERS),1)
        CLIENT_CFLAGS += -DCURL_STATICLIB -I$(CURLHDIR)
        ifeq ($(ARCH),x86_64)
          CLIENT_LIBS += $(LIBSDIR)/win64/libcurl.a
        else
          CLIENT_LIBS += $(LIBSDIR)/win32/libcurl.a
        endif
      else
		CLIENT_CFLAGS += $(CURL_CFLAGS)
        CLIENT_LIBS += $(CURL_LIBS)
      endif
    endif
  endif

  # libmingw32 must be linked before libSDLmain
  CLIENT_LIBS += -lmingw32
  RENDERER_LIBS += -lmingw32

  ifeq ($(USE_LOCAL_HEADERS),1)
    CLIENT_CFLAGS += -I$(SDLHDIR)/include
    ifeq ($(ARCH), x86)
      CLIENT_LIBS += $(LIBSDIR)/win32/libSDL2main.a $(LIBSDIR)/win32/libSDL2.dll.a
      RENDERER_LIBS += $(LIBSDIR)/win32/libSDL2main.a $(LIBSDIR)/win32/libSDL2.dll.a
      SDLDLL=SDL2.dll
      CLIENT_EXTRA_FILES += $(LIBSDIR)/win32/SDL2.dll
    else
      CLIENT_LIBS += $(LIBSDIR)/win64/libSDL264main.a  $(LIBSDIR)/win64/libSDL264.dll.a
      RENDERER_LIBS += $(LIBSDIR)/win64/libSDL264main.a $(LIBSDIR)/win64/libSDL264.dll.a
      SDLDLL=SDL264.dll
      CLIENT_EXTRA_FILES += $(LIBSDIR)/win64/SDL264.dll
    endif
  else
    CLIENT_CFLAGS += $(SDL_CFLAGS)
    CLIENT_LIBS += $(SDL_LIBS)
    RENDERER_LIBS += $(SDL_LIBS)
    SDLDLL=SDL2.dll
  endif

else # ifdef MINGW

#############################################################################
# SETUP AND BUILD -- FREEBSD
#############################################################################

ifeq ($(PLATFORM),freebsd)

  # flags
  BASE_CFLAGS = $(shell env MACHINE_ARCH=$(ARCH) make -f /dev/null -VCFLAGS) \
    -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -DUSE_ICON -DMAP_ANONYMOUS=MAP_ANON
  CLIENT_CFLAGS += $(SDL_CFLAGS)
  HAVE_VM_COMPILED = true

  OPTIMIZEVM = -O3
  OPTIMIZE = $(OPTIMIZEVM) -ffast-math

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  #SHLIBLDFLAGS=-shared $(LDFLAGS)
  SHLIBLDFLAGS=-shared

  THREAD_LIBS=-lpthread
  # don't need -ldl (FreeBSD)
  LIBS=-lm
  GRANGER_LIBS = -lm

  CLIENT_LIBS =

  CLIENT_LIBS += $(SDL_LIBS)
  RENDERER_LIBS = $(SDL_LIBS) -lGL

  # optional features/libraries
  ifeq ($(USE_OPENAL),1)
    ifeq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LIBS += $(THREAD_LIBS) $(OPENAL_LIBS)
    endif
  endif

  ifeq ($(USE_CURL),1)
    CLIENT_CFLAGS += $(CURL_CFLAGS)
    ifeq ($(USE_CURL_DLOPEN),1)
      CLIENT_LIBS += $(CURL_LIBS)
    endif
  endif

  # cross-compiling tweaks
  ifeq ($(ARCH),x86)
    ifeq ($(CROSS_COMPILING),1)
      BASE_CFLAGS += -m32
    endif
  endif
  ifeq ($(ARCH),x86_64)
    ifeq ($(CROSS_COMPILING),1)
      BASE_CFLAGS += -m64
    endif
  endif
else # ifeq freebsd

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################
  BASE_CFLAGS=
  OPTIMIZE = -O3

  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared

endif #Linux
endif #darwin
endif #MINGW
endif #FreeBSD

ifndef CC
  CC=gcc
endif

ifndef RANLIB
  RANLIB=ranlib
endif

ifneq ($(HAVE_VM_COMPILED),true)
  BASE_CFLAGS += -DNO_VM_COMPILED
  BUILD_GAME_QVM=0
endif

TARGETS =

ifndef FULLBINEXT
  FULLBINEXT=$(BINEXT)
endif

ifndef SHLIBNAME
  SHLIBNAME=.$(SHLIBEXT)
endif

ifneq ($(BUILD_SERVER),0)
  TARGETS += $(B)/$(SERVERBIN)$(FULLBINEXT)
endif

ifneq ($(BUILD_CLIENT),0)
  ifneq ($(USE_RENDERER_DLOPEN),0)
    TARGETS += $(B)/$(CLIENTBIN)$(FULLBINEXT) $(B)/renderer_opengl1$(SHLIBNAME)
    ifneq ($(BUILD_RENDERER_OPENGL2),0)
      TARGETS += $(B)/renderer_opengl2$(SHLIBNAME)
    endif
  else
    TARGETS += $(B)/$(CLIENTBIN)$(FULLBINEXT)
    ifneq ($(BUILD_RENDERER_OPENGL2),0)
      TARGETS += $(B)/$(CLIENTBIN)_opengl2$(FULLBINEXT)
    endif
  endif
endif

ifneq ($(BUILD_GAME_SO),0)
  TARGETS += \
    $(B)/$(BASEGAME)/cgame$(SHLIBNAME) \
    $(B)/$(BASEGAME)/game$(SHLIBNAME) \
    $(B)/$(BASEGAME)/ui$(SHLIBNAME)
endif

ifneq ($(BUILD_GAME_QVM),0)
  TARGETS += \
    $(B)/$(BASEGAME)/vm/cgame.qvm \
    $(B)/$(BASEGAME)/vm/game.qvm \
    $(B)/$(BASEGAME)/vm/ui.qvm \
	$(B)/$(BASEGAME)/vms-gpp-$(VERSION).pk3
endif

ifneq ($(BUILD_GAME_QVM_11),0)
  TARGETS += \
    $(B)/$(BASEGAME)_11/vm/cgame.qvm \
    $(B)/$(BASEGAME)_11/vm/ui.qvm \
	$(B)/$(BASEGAME)_11/vms-1.1.0-$(VERSION).pk3
endif

ifneq ($(BUILD_DATA_PK3),0)
  TARGETS += \
    $(B)/$(BASEGAME)/data-$(VERSION).pk3
endif

ifeq ($(USE_OPENAL),1)
  CLIENT_CFLAGS += -DUSE_OPENAL
  ifeq ($(USE_OPENAL_DLOPEN),1)
    CLIENT_CFLAGS += -DUSE_OPENAL_DLOPEN
  endif
endif

ifeq ($(USE_CURL),1)
  CLIENT_CFLAGS += -DUSE_CURL
  ifeq ($(USE_CURL_DLOPEN),1)
    CLIENT_CFLAGS += -DUSE_CURL_DLOPEN
  endif
endif

ifeq ($(USE_VOIP),1)
  CLIENT_CFLAGS += -DUSE_VOIP
  SERVER_CFLAGS += -DUSE_VOIP
  NEED_OPUS=1
endif

ifeq ($(USE_CODEC_OPUS),1)
  CLIENT_CFLAGS += -DUSE_CODEC_OPUS
  NEED_OPUS=1
endif

ifeq ($(NEED_OPUS),1)
  ifeq ($(USE_INTERNAL_OPUS),1)
    OPUS_CFLAGS = -DOPUS_BUILD -DHAVE_LRINTF -DFLOATING_POINT -DFLOAT_APPROX -DUSE_ALLOCA -D __OPTIMIZE__ \
      -I$(OPUSDIR)/include -I$(OPUSDIR)/celt -I$(OPUSDIR)/silk \
      -I$(OPUSDIR)/silk/float -I$(OPUSFILEDIR)/include
  else
    OPUS_CFLAGS ?= $(shell pkg-config --silence-errors --cflags opusfile opus || true)
    OPUS_LIBS ?= $(shell pkg-config --silence-errors --libs opusfile opus || echo -lopusfile -lopus)
  endif
  CLIENT_CFLAGS += $(OPUS_CFLAGS)
  CLIENT_LIBS += $(OPUS_LIBS)
  NEED_OGG=1
endif

ifeq ($(USE_CODEC_VORBIS),1)
  CLIENT_CFLAGS += -DUSE_CODEC_VORBIS
  ifeq ($(USE_INTERNAL_VORBIS),1)
    CLIENT_CFLAGS += -I$(VORBISDIR)/include -I$(VORBISDIR)/lib
  else
    VORBIS_CFLAGS ?= $(shell pkg-config --silence-errors --cflags vorbisfile vorbis || true)
    VORBIS_LIBS ?= $(shell pkg-config --silence-errors --libs vorbisfile vorbis || echo -lvorbisfile -lvorbis)
  endif
  CLIENT_CFLAGS += $(VORBIS_CFLAGS)
  CLIENT_LIBS += $(VORBIS_LIBS)
  NEED_OGG=1
endif

#-bbq
ifeq ($(USE_RESTCLIENT),1)
  CLIENT_CFLAGS += -DUSE_RESTCLIENT -I$(RESTDIR)
endif

ifeq ($(NEED_OGG),1)
  ifeq ($(USE_INTERNAL_OGG),1)
    OGG_CFLAGS = -I$(OGGDIR)/include
  else
    OGG_CFLAGS ?= $(shell pkg-config --silence-errors --cflags ogg || true)
    OGG_LIBS ?= $(shell pkg-config --silence-errors --libs ogg || echo -logg)
  endif
  CLIENT_CFLAGS += $(OGG_CFLAGS)
  CLIENT_LIBS += $(OGG_LIBS)
endif

ifeq ($(USE_RENDERER_DLOPEN),1)
  CLIENT_CFLAGS += -DUSE_RENDERER_DLOPEN
endif

ifeq ($(USE_MUMBLE),1)
  CLIENT_CFLAGS += -DUSE_MUMBLE
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
  ZLIB_CFLAGS = -DNO_GZIP -I$(ZDIR)
else
  ZLIB_CFLAGS ?= $(shell pkg-config --silence-errors --cflags zlib || true)
  ZLIB_LIBS ?= $(shell pkg-config --silence-errors --libs zlib || echo -lz)
endif
BASE_CFLAGS += $(ZLIB_CFLAGS)
LIBS += $(ZLIB_LIBS)

ifeq ($(USE_INTERNAL_JPEG),1)
  BASE_CFLAGS += -DUSE_INTERNAL_JPEG
  BASE_CFLAGS += -I$(JPDIR)
else
  # IJG libjpeg doesn't have pkg-config, but libjpeg-turbo uses libjpeg.pc;
  # we fall back to hard-coded answers if libjpeg.pc is unavailable
  JPEG_CFLAGS ?= $(shell pkg-config --silence-errors --cflags libjpeg || true)
  JPEG_LIBS ?= $(shell pkg-config --silence-errors --libs libjpeg || echo -ljpeg)
  BASE_CFLAGS += $(JPEG_CFLAGS)
  RENDERER_LIBS += $(JPEG_LIBS)
endif

ifeq ($(USE_FREETYPE),1)
  FREETYPE_CFLAGS ?= $(shell pkg-config --silence-errors --cflags freetype2 || true)
  FREETYPE_LIBS ?= $(shell pkg-config --silence-errors --libs freetype2 || echo -lfreetype)

  BASE_CFLAGS += -DBUILD_FREETYPE $(FREETYPE_CFLAGS)
  RENDERER_LIBS += $(FREETYPE_LIBS)
endif

ifeq ("$(CC)", $(findstring "$(CC)", "clang" "clang++"))
  BASE_CFLAGS += -Qunused-arguments
endif

ifdef DEFAULT_BASEDIR
  BASE_CFLAGS += -DDEFAULT_BASEDIR=\\\"$(DEFAULT_BASEDIR)\\\"
endif

ifeq ($(USE_LOCAL_HEADERS),1)
  BASE_CFLAGS += -DUSE_LOCAL_HEADERS
endif

ifeq ($(BUILD_STANDALONE),1)
  BASE_CFLAGS += -DSTANDALONE
endif

ifeq ($(GENERATE_DEPENDENCIES),1)
  DEPEND_CFLAGS = -MMD
else
  DEPEND_CFLAGS =
endif

ifeq ($(NO_STRIP),1)
  STRIP_FLAG =
else
  STRIP_FLAG = -s
endif

BASE_CFLAGS += -DPRODUCT_VERSION=\\\"$(VERSION)\\\"

ifeq ($(V),1)
echo_cmd=@:
Q=
else
echo_cmd=@echo
Q=@
endif

EXEC_CC = $(CC) ${1} -o ${2} -c ${3}
EXEC_CXX = $(CXX) -std=c++1y ${CXXFLAGS} ${1} -o ${2} -c ${3}

# TREMULOUS CLIENT
CC_FLAGS=${NOTSHLIBCFLAGS} ${CFLAGS} ${CLIENT_CFLAGS} ${OPTIMIZE}
define DO_CC
$(echo_cmd) "CC $<"
$(Q)$(call EXEC_CC,-std=gnu99 ${CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,tremulous,${CC_FLAGS},$@,$<)
endef

define DO_CXX
$(echo_cmd) "CXX $<"
$(Q)$(call EXEC_CXX,${CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CXX,tremulous,${CC_FLAGS},$@,$<)
endef

##########################################
# Renderers
##########################################
# Common Rendering Code
define DO_RENDERER_COMMON_CC
$(echo_cmd) "RENDERER_COMMON_CC $<"
$(Q)$(call EXEC_CC,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,renderer_common,${REF_CC_FLAGS},$@,$<)
endef
define DO_RENDERER_COMMON_CXX
$(echo_cmd) "RENDERER_COMMON_CXX $<"
$(Q)$(call EXEC_CXX,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CXX,renderer_common,${REF_CC_FLAGS},$@,$<)
endef
##########################################
# Renderers
##########################################
# OpenGL 1 Renderer
REF_CC_FLAGS=${SHLIBCFLAGS} ${CFLAGS} ${CLIENT_CFLAGS} ${OPTIMIZE}
define DO_RENDERERGL1_CC
$(echo_cmd) "GL1_RENDERER_CC $<"
$(Q)$(call EXEC_CC,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,opengl1,${REF_CC_FLAGS},$@,$<)
endef
define DO_RENDERERGL1_CXX
$(echo_cmd) "GL1_RENDERER_CXX $<"
$(Q)$(call EXEC_CXX,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CXX,opengl1,${REF_CC_FLAGS},$@,$<)
endef
##########################################
# Renderers
##########################################
# OpenGL 2 Renderer
define DO_RENDERERGL2_CC
$(echo_cmd) "GL2_RENDERER_CC $<"
$(Q)$(call EXEC_CC,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,opengl2,${REF_CC_FLAGS},$@,$<)
endef
define DO_RENDERERGL2_CXX
$(echo_cmd) "RENDERERGL2_CXX $<"
$(Q)$(call EXEC_CXX,${REF_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CXX,opengl2,${REF_CC_FLAGS},$@,$<)
endef

define DO_REF_STR
$(echo_cmd) "REF_STR $<"
$(Q)rm -f $@
$(Q)echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
$(Q)cat $< | sed 's/^/\"/;s/$$/\\n\"/' >> $@
mkdir -p $(B)/glsl
cp $@ $(B)/glsl/
$(Q)echo ";" >> $@
endef

ifeq ($(GENERATE_DEPENDENCIES),1)
  DO_QVM_DEP=cat $(@:%.o=%.d) | sed -e 's/\.o/\.asm/g' >> $(@:%.o=%.d)
endif

SHLIB_CC_FLAGS=${BASEGAME_CFLAGS} ${SHLIBCFLAGS} ${CFLAGS} ${OPTIMIZEVM}
define DO_SHLIB_CC
$(echo_cmd) "SHLIB_CC $<"
$(Q)$(call EXEC_CC,${SHLIB_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,qcommon,${SHLIB_CC_FLAGS},$@,$<)
$(Q)$(DO_QVM_DEP)
endef

GAME_CC_FLAGS=${BASEGAME_CFLAGS} ${SHLIBCFLAGS} ${CFLAGS} ${OPTIMIZEVM}
define DO_GAME_CC
$(echo_cmd) "GAME_CC $<"
$(Q)$(call EXEC_CC,-DGAME ${GAME_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,game,-DGAME ${GAME_CC_FLAGS},$@,$<)
$(Q)$(DO_QVM_DEP)
endef

define DO_CGAME_CC
$(echo_cmd) "CGAME_CC $<"
$(Q)$(call EXEC_CC,-DCGAME ${GAME_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,cgame,-DCGAME ${GAME_CC_FLAGS},$@,$<)
$(Q)$(DO_QVM_DEP)
endef

define DO_UI_CC
$(echo_cmd) "UI_CC $<"
$(Q)$(call EXEC_CC,-DUI ${GAME_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,ui,-DUI ${GAME_CC_FLAGS},$@,$<)
$(Q)$(DO_QVM_DEP)
endef

AS_FLAGS=${CFLAGS} ${OPTIMIZE} -x assembler-with-cpp
define DO_AS
$(echo_cmd) "AS $<"
$(Q)$(call EXEC_CC,${AS_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,tremulous,${AS_FLAGS},$@,$<)
endef

define DO_DED_AS
$(echo_cmd) "AS $<"
$(Q)$(call EXEC_CC,${AS_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,tremded,${AS_FLAGS},$@,$<)
endef

DED_CC_FLAGS=-DDEDICATED ${NOTSHLIBCFLAGS} ${CFLAGS} ${SERVER_CFLAGS} ${OPTIMIZE}
define DO_DED_CC
$(echo_cmd) "DED_CC $<"
$(Q)$(call EXEC_CC,-std=gnu99 ${DED_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CC,tremded,${DED_CC_FLAGS},$@,$<)
endef

define DO_DED_CXX
$(echo_cmd) "DED_CXX $<"
$(Q)$(call EXEC_CXX,${DED_CC_FLAGS},'$@','$<')
$(Q)$(call LOG_CXX,tremded,${DED_CC_FLAGS},$@,$<)
endef

define DO_WINDRES
$(echo_cmd) "WINDRES $<"
$(Q)$(WINDRES) -i $< -o $@
endef


#############################################################################
# MAIN TARGETS
#############################################################################

default: release
all: debug release

debug:
	@$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
      CXXFLAGS="$(BASE_CFLAGS) $(CXXFLAGS)" \
	  OPTIMIZE="$(DEBUG_CFLAGS)" OPTIMIZEVM="$(DEBUG_CFLAGS)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)
release:
	@$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(BASE_CFLAGS) $(DEPEND_CFLAGS)" \
      CXXFLAGS="$(BASE_CFLAGS) $(CXXFLAGS)" \
	  OPTIMIZE="-DNDEBUG $(OPTIMIZE)" OPTIMIZEVM="-DNDEBUG $(OPTIMIZEVM)" \
	  CLIENT_CFLAGS="$(CLIENT_CFLAGS)" SERVER_CFLAGS="$(SERVER_CFLAGS)" V=$(V)

ifneq ($(call bin_path, tput),)
  TERM_COLUMNS=$(shell if c=`tput cols`; then echo $$(($$c-4)); else echo 76; fi)
else
  TERM_COLUMNS=76
endif

define ADD_COPY_TARGET
TARGETS += $2
$2: $1
	$(echo_cmd) "CP $$<"
	@cp $1 $2
endef

# These functions allow us to generate rules for copying a list of files
# into the base directory of the build; this is useful for bundling libs,
# README files or whatever else
define GENERATE_COPY_TARGETS
$(foreach FILE,$1, \
  $(eval $(call ADD_COPY_TARGET, \
    $(FILE), \
    $(addprefix $(B)/,$(notdir $(FILE))))))
endef

$(call GENERATE_COPY_TARGETS,$(EXTRA_FILES))

ifneq ($(BUILD_CLIENT),0)
  $(call GENERATE_COPY_TARGETS,$(CLIENT_EXTRA_FILES))
endif

NAKED_TARGETS=$(shell echo $(TARGETS) | sed -e "s!$(B)/!!g")

print_list=-@for i in $(1); \
     do \
             echo "    $$i"; \
     done

ifneq ($(call bin_path, fmt),)
  print_wrapped=@echo $(1) | fmt -w $(TERM_COLUMNS) | sed -e "s/^\(.*\)$$/    \1/"
else
  print_wrapped=$(print_list)
endif

#$(B)/compile_commands.json: $(B)/compile_commands.txt
#	sed -i -e "$$ ! s/}/},/" $<
#	echo '[' >$@
#	cat $< >>$@
#	echo ']' >>$@

# Create the build directories, check libraries and print out
# an informational message, then start building
targets: makedirs
	@echo ""
	@echo "Building in $(B):"
	@echo "  PLATFORM: $(PLATFORM)"
	@echo "  ARCH: $(ARCH)"
	@echo "  VERSION: $(VERSION)"
	@echo "  COMPILE_PLATFORM: $(COMPILE_PLATFORM)"
	@echo "  COMPILE_ARCH: $(COMPILE_ARCH)"
	@echo "  CC: $(CC)"
	@echo "  CXX: $(CXX)"
	@echo "  TOOLS_CC $(TOOLS_CC)"
ifeq ($(PLATFORM),mingw32)
	@echo "  WINDRES: $(WINDRES)"
endif
	@echo ""
	@echo "  CFLAGS:"
	$(call print_wrapped, $(CFLAGS) $(OPTIMIZE))
	@echo ""
	@echo "  CXXFLAGS:"
	$(call print_wrapped, $(CXXFLAGS) $(OPTIMIZE))
	@echo ""
	@echo "  CLIENT_CFLAGS:"
	$(call print_wrapped, $(CLIENT_CFLAGS))
	@echo ""
	@echo "  CLIENT_CXXFLAGS:"
	$(call print_wrapped, $(CLIENT_CXXFLAGS))
	@echo ""
	@echo "  SERVER_CFLAGS:"
	$(call print_wrapped, $(SERVER_CFLAGS))
	@echo ""
	@echo "  SERVER_CXXFLAGS:"
	$(call print_wrapped, $(SERVER_CXXFLAGS))
	@echo ""
	@echo "  LDFLAGS:"
	$(call print_wrapped, $(LDFLAGS))
	@echo ""
	@echo "  LIBS:"
	$(call print_wrapped, $(LIBS))
	@echo ""
	@echo "  CLIENT_LIBS:"
	$(call print_wrapped, $(CLIENT_LIBS))
	@echo ""
	@echo "  Output:"
	$(call print_list, $(NAKED_TARGETS))
	@echo ""
	@$(MAKE) $(TARGETS) $(B).zip V=$(V)

$(B).zip: $(TARGETS)
ifeq ($(PLATFORM),darwin)
	@("./make-macosx-app.sh" release $(ARCH); if [ "$$?" -eq 0 ] && [ -d "$(B)/Tremulous.app" ]; then rm -f $@; cd $(B) && zip --symlinks -r9 ../../$@ GPL COPYING CC `find "Tremulous.app" -print | sed -e "s!$(B)/!!g"`; else rm -f $@; cd $(B) && zip -r9 ../../$@ $(NAKED_TARGETS); fi)
else
	@rm -f $@
	@(cd $(B) && zip -r9 ../../$@ $(NAKED_TARGETS))
endif

makedirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/lua ]; then $(MKDIR) $(B)/lua;fi
	@if [ ! -d $(B)/script ]; then $(MKDIR) $(B)/script;fi
	@if [ ! -d $(B)/script/rapidjson ]; then $(MKDIR) $(B)/script/rapidjson;fi
	@if [ ! -d $(B)/nettle ]; then $(MKDIR) $(B)/nettle;fi
	@if [ ! -d $(B)/semver ]; then $(MKDIR) $(B)/semver;fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
	@if [ ! -d $(B)/client/opus ];then $(MKDIR) $(B)/client/opus;fi
	@if [ ! -d $(B)/client/vorbis ];then $(MKDIR) $(B)/client/vorbis;fi
	@if [ ! -d $(B)/client/restclient ];then $(MKDIR) $(B)/client/restclient;fi
	@if [ ! -d $(B)/ded ];then $(MKDIR) $(B)/ded;fi
	@if [ ! -d $(B)/renderercommon ];then $(MKDIR) $(B)/renderercommon;fi
	@if [ ! -d $(B)/renderergl1 ];then $(MKDIR) $(B)/renderergl1;fi
	@if [ ! -d $(B)/renderergl2 ];then $(MKDIR) $(B)/renderergl2;fi
	@if [ ! -d $(B)/renderergl2/glsl ];then $(MKDIR) $(B)/renderergl2/glsl;fi
	@if [ ! -d $(B)/$(BASEGAME) ];then $(MKDIR) $(B)/$(BASEGAME);fi
	@if [ ! -d $(B)/$(BASEGAME)/cgame ];then $(MKDIR) $(B)/$(BASEGAME)/cgame;fi
	@if [ ! -d $(B)/$(BASEGAME)/game ];then $(MKDIR) $(B)/$(BASEGAME)/game;fi
	@if [ ! -d $(B)/$(BASEGAME)/ui ];then $(MKDIR) $(B)/$(BASEGAME)/ui;fi
	@if [ ! -d $(B)/$(BASEGAME)/qcommon ];then $(MKDIR) $(B)/$(BASEGAME)/qcommon;fi
	@if [ ! -d $(B)/$(BASEGAME)/11 ];then $(MKDIR) $(B)/$(BASEGAME)/11;fi
	@if [ ! -d $(B)/$(BASEGAME)/11/cgame ];then $(MKDIR) $(B)/$(BASEGAME)/11/cgame;fi
	@if [ ! -d $(B)/$(BASEGAME)/11/ui ];then $(MKDIR) $(B)/$(BASEGAME)/11/ui;fi
	@if [ ! -d $(B)/$(BASEGAME)/vm ];then $(MKDIR) $(B)/$(BASEGAME)/vm;fi
	@if [ ! -d $(B)/$(BASEGAME)_11 ];then $(MKDIR) $(B)/$(BASEGAME)_11;fi
	@if [ ! -d $(B)/$(BASEGAME)_11/vm ];then $(MKDIR) $(B)/$(BASEGAME)_11/vm;fi
	@if [ ! -d $(B)/granger.dir ];then $(MKDIR) $(B)/granger.dir;fi
	@if [ ! -d $(B)/granger.dir/src ];then $(MKDIR) $(B)/granger.dir/src;fi
	@if [ ! -d $(B)/granger.dir/src/lua ];then $(MKDIR) $(B)/granger.dir/src/lua;fi
	@if [ ! -d $(B)/granger.dir/src/premake ];then $(MKDIR) $(B)/granger.dir/src/premake;fi
	@if [ ! -d $(B)/granger.dir/src/nettle ];then $(MKDIR) $(B)/granger.dir/src/nettle;fi
	@if [ ! -d $(B)/tools ];then $(MKDIR) $(B)/tools;fi
	@if [ ! -d $(B)/tools/asm ];then $(MKDIR) $(B)/tools/asm;fi
	@if [ ! -d $(B)/tools/etc ];then $(MKDIR) $(B)/tools/etc;fi
	@if [ ! -d $(B)/tools/rcc ];then $(MKDIR) $(B)/tools/rcc;fi
	@if [ ! -d $(B)/tools/cpp ];then $(MKDIR) $(B)/tools/cpp;fi
	@if [ ! -d $(B)/tools/lburg ];then $(MKDIR) $(B)/tools/lburg;fi

#############################################################################
# QVM BUILD TOOLS
#############################################################################

ifndef TOOLS_CC
  # A compiler which probably produces native binaries
  TOOLS_CC = gcc
  #$(CC)
endif

ifndef YACC
  YACC = yacc
endif

TOOLS_OPTIMIZE = -g -Wall -fno-strict-aliasing
TOOLS_CFLAGS += $(TOOLS_OPTIMIZE) \
                -DTEMPDIR=\"$(TEMPDIR)\" -DSYSTEM=\"\" \
				-I$(MOUNT_DIR) \
                -I$(Q3LCCSRCDIR) \
                -I$(LBURGDIR)
TOOLS_LIBS =
TOOLS_LDFLAGS =

ifeq ($(GENERATE_DEPENDENCIES),1)
  TOOLS_CFLAGS += -MMD
endif

define DO_YACC
$(echo_cmd) "YACC $<"
$(Q)$(YACC) $<
$(Q)mv -f y.tab.c $@
endef

define DO_TOOLS_CC
$(echo_cmd) "TOOLS_CC $<"
$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) -o $@ -c $<
endef

define DO_TOOLS_CC_DAGCHECK
$(echo_cmd) "TOOLS_CC_DAGCHECK $<"
$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) -Wno-unused -o $@ -c $<
endef

LBURG       = $(B)/tools/lburg/lburg$(TOOLS_BINEXT)
DAGCHECK_C  = $(B)/tools/rcc/dagcheck.c
Q3RCC       = $(B)/tools/q3rcc$(TOOLS_BINEXT)
Q3CPP       = $(B)/tools/q3cpp$(TOOLS_BINEXT)
Q3LCC       = $(B)/tools/q3lcc$(TOOLS_BINEXT)
Q3ASM       = $(B)/tools/q3asm$(TOOLS_BINEXT)

LBURGOBJ= \
  $(B)/tools/lburg/lburg.o \
  $(B)/tools/lburg/gram.o

# override GNU Make built-in rule for converting gram.y to gram.c
%.c: %.y
ifeq ($(USE_YACC),1)
	$(DO_YACC)
endif

$(B)/tools/lburg/%.o: $(LBURGDIR)/%.c
	$(DO_TOOLS_CC)

$(LBURG): $(LBURGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) -std=gnu99 $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3RCCOBJ = \
  $(B)/tools/rcc/alloc.o \
  $(B)/tools/rcc/bind.o \
  $(B)/tools/rcc/bytecode.o \
  $(B)/tools/rcc/dag.o \
  $(B)/tools/rcc/dagcheck.o \
  $(B)/tools/rcc/decl.o \
  $(B)/tools/rcc/enode.o \
  $(B)/tools/rcc/error.o \
  $(B)/tools/rcc/event.o \
  $(B)/tools/rcc/expr.o \
  $(B)/tools/rcc/gen.o \
  $(B)/tools/rcc/init.o \
  $(B)/tools/rcc/inits.o \
  $(B)/tools/rcc/input.o \
  $(B)/tools/rcc/lex.o \
  $(B)/tools/rcc/list.o \
  $(B)/tools/rcc/main.o \
  $(B)/tools/rcc/null.o \
  $(B)/tools/rcc/output.o \
  $(B)/tools/rcc/prof.o \
  $(B)/tools/rcc/profio.o \
  $(B)/tools/rcc/simp.o \
  $(B)/tools/rcc/stmt.o \
  $(B)/tools/rcc/string.o \
  $(B)/tools/rcc/sym.o \
  $(B)/tools/rcc/symbolic.o \
  $(B)/tools/rcc/trace.o \
  $(B)/tools/rcc/tree.o \
  $(B)/tools/rcc/types.o

$(DAGCHECK_C): $(LBURG) $(Q3LCCSRCDIR)/dagcheck.md
	$(echo_cmd) "LBURG $(Q3LCCSRCDIR)/dagcheck.md"
	$(Q)$(LBURG) $(Q3LCCSRCDIR)/dagcheck.md $@

$(B)/tools/rcc/dagcheck.o: $(DAGCHECK_C)
	$(DO_TOOLS_CC_DAGCHECK)

$(B)/tools/rcc/%.o: $(Q3LCCSRCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3RCC): $(Q3RCCOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3CPPOBJ = \
  $(B)/tools/cpp/cpp.o \
  $(B)/tools/cpp/lex.o \
  $(B)/tools/cpp/nlist.o \
  $(B)/tools/cpp/tokens.o \
  $(B)/tools/cpp/macro.o \
  $(B)/tools/cpp/eval.o \
  $(B)/tools/cpp/include.o \
  $(B)/tools/cpp/hideset.o \
  $(B)/tools/cpp/getopt.o \
  $(B)/tools/cpp/unix.o

$(B)/tools/cpp/%.o: $(Q3CPPDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3CPP): $(Q3CPPOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

Q3LCCOBJ = \
	$(B)/tools/etc/lcc.o \
	$(B)/tools/etc/bytecode.o

$(B)/tools/etc/%.o: $(Q3LCCETCDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3LCC): $(Q3LCCOBJ) $(Q3RCC) $(Q3CPP)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $(Q3LCCOBJ) $(TOOLS_LIBS)

define DO_Q3LCC
$(echo_cmd) "Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -o $@ $<
endef

define DO_CGAME_Q3LCC
$(echo_cmd) "CGAME_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DCGAME -o $@ $<
endef

define DO_GAME_Q3LCC
$(echo_cmd) "GAME_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DGAME -o $@ $<
endef

define DO_UI_Q3LCC
$(echo_cmd) "UI_Q3LCC $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DUI -o $@ $<
endef

define DO_CGAME_Q3LCC_11
$(echo_cmd) "CGAME_Q3LCC_11 $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DMODULE_INTERFACE_11 -DCGAME -o $@ $<
endef

define DO_UI_Q3LCC_11
$(echo_cmd) "UI_Q3LCC_11 $<"
$(Q)$(Q3LCC) $(BASEGAME_CFLAGS) -DMODULE_INTERFACE_11 -DUI -o $@ $<
endef

Q3ASMOBJ = \
  $(B)/tools/asm/q3asm.o \
  $(B)/tools/asm/cmdlib.o

$(B)/tools/asm/%.o: $(Q3ASMDIR)/%.c
	$(DO_TOOLS_CC)

$(Q3ASM): $(Q3ASMOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(TOOLS_CC) $(TOOLS_CFLAGS) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

#############################################################################
# GRANGER
#############################################################################

ifeq ($(ARCH),x86_64)
    ARCHFLAG=-m64
else
ifeq ($(ARCH),x86)
    ARCHFLAG=-m32
endif
endif

GRANGER_CFLAGS += $(ARCHFLAG) -fPIC -fpic $(LUACFLAGS) 

ifeq ($(PLATFORM),darwin)
GRANGER_CFLAGS += -DLUA_USE_MACOSX
else
ifeq ($(PLATFORM),linux)
GRANGER_CFLAGS += -DLUA_USE_LINUX
endif
endif

GRANGEROBJ = \
	$(B)/granger.dir/src/lnettlelib.o \
	$(B)/granger.dir/src/main.o \
	$(B)/granger.dir/src/strvec.o \
	$(B)/granger.dir/src/lua/lapi.o \
	$(B)/granger.dir/src/lua/lauxlib.o \
	$(B)/granger.dir/src/lua/lbaselib.o \
	$(B)/granger.dir/src/lua/lbitlib.o \
	$(B)/granger.dir/src/lua/lcode.o \
	$(B)/granger.dir/src/lua/lcorolib.o \
	$(B)/granger.dir/src/lua/lctype.o \
	$(B)/granger.dir/src/lua/ldblib.o \
	$(B)/granger.dir/src/lua/ldebug.o \
	$(B)/granger.dir/src/lua/ldo.o \
	$(B)/granger.dir/src/lua/ldump.o \
	$(B)/granger.dir/src/lua/lfunc.o \
	$(B)/granger.dir/src/lua/lgc.o \
	$(B)/granger.dir/src/lua/linit.o \
	$(B)/granger.dir/src/lua/liolib.o \
	$(B)/granger.dir/src/lua/llex.o \
	$(B)/granger.dir/src/lua/lmathlib.o \
	$(B)/granger.dir/src/lua/lmem.o \
	$(B)/granger.dir/src/lua/loadlib.o \
	$(B)/granger.dir/src/lua/lobject.o \
	$(B)/granger.dir/src/lua/lopcodes.o \
	$(B)/granger.dir/src/lua/loslib.o \
	$(B)/granger.dir/src/lua/lparser.o \
	$(B)/granger.dir/src/lua/lstate.o \
	$(B)/granger.dir/src/lua/lstring.o \
	$(B)/granger.dir/src/lua/lstrlib.o \
	$(B)/granger.dir/src/lua/ltable.o \
	$(B)/granger.dir/src/lua/ltablib.o \
	$(B)/granger.dir/src/lua/ltm.o \
	$(B)/granger.dir/src/lua/lundump.o \
	$(B)/granger.dir/src/lua/lutf8lib.o \
	$(B)/granger.dir/src/lua/lvm.o \
	$(B)/granger.dir/src/lua/lzio.o \
	$(B)/granger.dir/src/premake/os_access.o \
	$(B)/granger.dir/src/premake/os_chdir.o \
	$(B)/granger.dir/src/premake/os_copyfile.o \
	$(B)/granger.dir/src/premake/os_elevate.o \
	$(B)/granger.dir/src/premake/os_getcwd.o \
	$(B)/granger.dir/src/premake/os_is64bit.o \
	$(B)/granger.dir/src/premake/os_isdir.o \
	$(B)/granger.dir/src/premake/os_isfile.o \
	$(B)/granger.dir/src/premake/os_match.o \
	$(B)/granger.dir/src/premake/os_mkdir.o \
	$(B)/granger.dir/src/premake/os_pathsearch.o \
	$(B)/granger.dir/src/premake/os_rmdir.o \
	$(B)/granger.dir/src/premake/os_stat.o \
	$(B)/granger.dir/src/premake/path_getabsolute.o \
	$(B)/granger.dir/src/premake/path_getrelative.o \
	$(B)/granger.dir/src/premake/path_isabsolute.o \
	$(B)/granger.dir/src/premake/path_join.o \
	$(B)/granger.dir/src/premake/path_normalize.o \
	$(B)/granger.dir/src/premake/path_translate.o \
	$(B)/granger.dir/src/premake/premake.o \
	$(B)/granger.dir/src/premake/string_endswith.o \
	$(B)/granger.dir/src/nettle/md5-compress.o \
	$(B)/granger.dir/src/nettle/md5.o \
	$(B)/granger.dir/src/nettle/sha256-compress.o \
	$(B)/granger.dir/src/nettle/sha256.o \
	$(B)/granger.dir/src/nettle/write-be32.o \
	$(B)/granger.dir/src/nettle/write-le32.o

define DO_GRANGER_CC
  $(echo_cmd) "GRANGER_CC $<"
  $(Q)$(call EXEC_CC,-std=gnu99 -DGRANGER ${GRANGER_CFLAGS} ${OPTIMIZE},'$@','$<')
  $(Q)$(call LOG_CC,granger,-std=gnu99 ${GRANGER_CFLAGS} ${OPTIMIZE},$@,$<)
endef

 define DO_GRANGER_CPP
  $(echo_cmd) "GRANGER_CC $<"
  $(Q)$(call EXEC_CC,-std=gnu99 -DGRANGER ${GRANGER_CFLAGS} ${OPTIMIZE},'$@','$<')
  $(Q)$(call LOG_CC,granger,-std=gnu99 ${GRANGER_CFLAGS} ${OPTIMIZE},$@,$<)
endef
 
$(B)/granger.dir/src/lua/%.o: $(LUADIR)/%.c
	$(DO_GRANGER_CC)

$(B)/granger.dir/src/premake/%.o: $(GRANGERDIR)/premake/%.c
	$(DO_GRANGER_CC)

$(B)/granger.dir/src/nettle/%.o: $(GRANGERDIR)/nettle/%.c
	$(DO_GRANGER_CC)

$(B)/granger.dir/src/%.o: $(GRANGERDIR)/%.c
	$(DO_GRANGER_CC)

$(B)/granger.dir/src/%.o: $(GRANGERDIR)/%.cpp
	$(DO_GRANGER_CPP)

$(B)/granger$(FULLBINEXT): $(GRANGEROBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(GRANGEROBJ) $(GRANGER_LIBS) 

ifneq ($(BUILD_GRANGER),0)
TARGETS += $(B)/granger$(FULLBINEXT)
endif

$(B)/scripts:
	rsync -rupE --exclude=".*" scripts $(B)

TARGETS += $(B)/scripts

$(B)/GPL:
	rsync GPL $(B)
$(B)/COPYING:
	rsync COPYING $(B)
$(B)/CC:
	rsync CC $(B)

TARGETS += $(B)/GPL $(B)/COPYING $(B)/CC

#############################################################################
# LUA
#############################################################################

#LUACFLAGS=-Wall -Wextra -fPIC -fpic
LUACFLAGS= $(ARCHFLAG) -fPIC -fpic

ifeq ($(PLATFORM),darwin)
LUACFLAGS += -DLUA_USE_MACOSX
else
ifeq ($(PLATFORM),linux)
LUACFLAGS += -DLUA_USE_LINUX
endif
endif

LUAOBJ = \
  $(B)/lua/lapi.o \
  $(B)/lua/lcode.o \
  $(B)/lua/lctype.o \
  $(B)/lua/ldebug.o \
  $(B)/lua/ldo.o \
  $(B)/lua/ldump.o \
  $(B)/lua/lfunc.o \
  $(B)/lua/lgc.o \
  $(B)/lua/llex.o \
  $(B)/lua/lmem.o \
  $(B)/lua/lobject.o \
  $(B)/lua/lopcodes.o \
  $(B)/lua/lparser.o \
  $(B)/lua/lstate.o \
  $(B)/lua/lstring.o \
  $(B)/lua/ltable.o \
  $(B)/lua/ltm.o \
  $(B)/lua/lundump.o \
  $(B)/lua/lvm.o \
  $(B)/lua/lzio.o \
  $(B)/lua/lauxlib.o \
  $(B)/lua/lbaselib.o \
  $(B)/lua/lbitlib.o \
  $(B)/lua/lcorolib.o \
  $(B)/lua/ldblib.o \
  $(B)/lua/liolib.o \
  $(B)/lua/lmathlib.o \
  $(B)/lua/loslib.o \
  $(B)/lua/lstrlib.o \
  $(B)/lua/ltablib.o \
  $(B)/lua/lutf8lib.o \
  $(B)/lua/loadlib.o \
  $(B)/lua/linit.o

define DO_LUA_CC
  $(echo_cmd) "LUA_CC $<"
  $(Q)$(call EXEC_CC,${LUACFLAGS} ${OPTIMIZE},'$@','$<')
  $(Q)$(call LOG_CC,lua,${LUACFLAGS} ${OPTIMIZE},$@,$<)
endef

LUACFLAGS += -I$(EXTERNAL_DIR)/lua-5.3.3/include -I$(EXTERNAL_DIR)/sol
CFLAGS += $(LUACFLAGS)
CXXFLAGS += $(LUACFLAGS)

$(B)/lua/%.o: $(LUADIR)/%.c
	$(DO_LUA_CC)

#############################################################################
# Script API
# FIXME Disabled for the time being
#############################################################################

SCRIPT_INCLUDES=-I$(MOUNT_DIR) -I$(EXTERNAL_DIR)/rapidjson -I$(EXTERNAL_DIR)/sol -I$(EXTERNAL_DIR)/nettle-3.3
define DO_SCRIPT_CXX
  $(echo_cmd) "SCRIPT_CXX $<"
  $(Q)$(call EXEC_CXX,${NOTSHLIBCFLAGS} ${SCRIPT_INCLUDES} ${LUACFLAGS} ${OPTIMIZE},'$@','$<')
  $(Q)$(call LOG_CXX,script,${NOTSHLIBCFLAGS} ${SCRIPT_INCLUDES} ${LUACFLAGS} ${OPTIMIZE},'$@','$<')
endef

define DO_SCRIPT_CC
  $(echo_cmd) "SCRIPT_CC $<"
  $(Q)$(call EXEC_CC,${NOTSHLIBCFLAGS} ${SCRIPT_INCLUDES} ${LUACFLAGS} ${OPTIMIZE},'$@','$<')
  $(Q)$(call LOG_CC,script,${NOTSHLIBCFLAGS} ${SCRIPT_INCLUDES} ${LUACFLAGS} ${OPTIMIZE},'$@','$<')
endef

SCRIPTOBJ = \
  $(B)/script/lnettlelib.o \
  $(B)/script/rapidjson/document.o \
  $(B)/script/rapidjson/rapidjson.o \
  $(B)/script/rapidjson/schema.o \
  $(B)/script/rapidjson/values.o

CFLAGS += -I${LUA_RAPIDJSONDIR} -I${SCRIPT_INCLUDES}

$(B)/script/%.o: $(SCRIPTDIR)/%.c
	$(DO_SCRIPT_CC)

$(B)/script/rapidjson/%.o: $(LUA_RAPIDJSONDIR)/%.cpp
	$(DO_SCRIPT_CXX)

#############################################################################
# Nettle
#############################################################################

NETTLECFLAGS=$(ARCHFLAG) -fPIC -fpic

define DO_NETTLE_CC
  $(echo_cmd) "NETTLE_CC $<"
  $(Q)$(call EXEC_CC,${NETTLECFLAGS},'$@','$<')
  $(Q)$(call LOG_CC,nettle,${NETTLECFLAGS},'$@','$<')
endef

NETTLEOBJ = \
  $(B)/nettle/bignum.o \
  $(B)/nettle/bignum-random.o \
  $(B)/nettle/bignum-random-prime.o \
  $(B)/nettle/buffer.o \
  $(B)/nettle/buffer-init.o \
  $(B)/nettle/gmp-glue.o \
  $(B)/nettle/mini-gmp.o \
  $(B)/nettle/pkcs1.o \
  $(B)/nettle/pkcs1-rsa-sha256.o \
  $(B)/nettle/realloc.o \
  $(B)/nettle/rsa.o \
  $(B)/nettle/rsa2sexp.o \
  $(B)/nettle/rsa-keygen.o \
  $(B)/nettle/rsa-sha256-sign.o \
  $(B)/nettle/rsa-sha256-verify.o \
  $(B)/nettle/rsa-sign.o \
  $(B)/nettle/rsa-verify.o \
  $(B)/nettle/sexp.o \
  $(B)/nettle/sexp-format.o \
  $(B)/nettle/sexp2bignum.o \
  $(B)/nettle/sexp2rsa.o \
  $(B)/nettle/sha256-compress.o \
  $(B)/nettle/sha256.o \
  $(B)/nettle/write-be32.o

CFLAGS += -I$(NETTLEDIR)

$(B)/nettle/%.o: $(NETTLEDIR)/nettle/%.c
	$(DO_NETTLE_CC)

#############################################################################
# Semver 
#############################################################################

SEMVERCFLAGS=$(ARCHFLAG) -fPIC -fpic -I$(SEMVERDIR)/src/include

define DO_SEMVER_CXX
  $(echo_cmd) "SEMVER_CC $<"
  $(CXX) -std=c++1y ${CXXFLAGS} ${SEMVERCFLAGS} -o $@ -c $<
endef

SEMVEROBJ = \
  $(B)/semver/semantic_version_v1.o \
  $(B)/semver/semantic_version_v2.o

CFLAGS += -I$(SEMVERDIR)/src/include

$(B)/semver/%.o: $(SEMVERDIR)/src/lib/%.cpp
	$(DO_SEMVER_CXX)

#############################################################################
# CLIENT/SERVER
#############################################################################

# FIXME: This should be CLIENT_INCLUDES and SERVER_INCLUDES to differentiate
# from GRANGER_INCLUDES etc.
CFLAGS += -I$(MOUNT_DIR)

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
  $(B)/client/cl_updates.o \
  $(B)/client/cl_rest.o \
  $(B)/client/cl_avi.o \
  \
  $(B)/client/q3_lauxlib.o \
  \
  $(B)/client/cm_load.o \
  $(B)/client/cm_patch.o \
  $(B)/client/cm_polylib.o \
  $(B)/client/cm_test.o \
  $(B)/client/cm_trace.o \
  \
  $(B)/client/cmd.o \
  $(B)/client/common.o \
  $(B)/client/crypto.o \
  $(B)/client/cvar.o \
  $(B)/client/files.o \
  $(B)/client/md4.o \
  $(B)/client/md5.o \
  $(B)/client/msg.o \
  $(B)/client/net_chan.o \
  $(B)/client/net_ip.o \
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
  $(B)/client/snd_codec_opus.o \
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
  $(B)/client/ioapi.o \
  $(B)/client/puff.o \
  $(B)/client/vm.o \
  $(B)/client/vm_interpreted.o \
  \
  \
  $(B)/client/sdl_input.o \
  $(B)/client/sdl_snd.o \
  \
  $(B)/client/con_log.o \
  $(B)/client/sys_main.o

ifdef MINGW
  Q3OBJ += \
    $(B)/client/con_passive.o
else
  Q3OBJ += \
    $(B)/client/con_tty.o
endif

Q3OBJ += $(LUAOBJ) $(SCRIPTOBJ) $(NETTLEOBJ) $(SEMVEROBJ)

Q3R2OBJ = \
  $(B)/renderergl2/tr_animation.o \
  $(B)/renderergl2/tr_backend.o \
  $(B)/renderergl2/tr_bsp.o \
  $(B)/renderergl2/tr_cmds.o \
  $(B)/renderergl2/tr_curve.o \
  $(B)/renderergl2/tr_dsa.o \
  $(B)/renderergl2/tr_extramath.o \
  $(B)/renderergl2/tr_extensions.o \
  $(B)/renderergl2/tr_fbo.o \
  $(B)/renderergl2/tr_flares.o \
  $(B)/renderergl2/tr_font.o \
  $(B)/renderergl2/tr_glsl.o \
  $(B)/renderergl2/tr_image.o \
  $(B)/renderergl2/tr_image_bmp.o \
  $(B)/renderergl2/tr_image_jpg.o \
  $(B)/renderergl2/tr_image_pcx.o \
  $(B)/renderergl2/tr_image_png.o \
  $(B)/renderergl2/tr_image_tga.o \
  $(B)/renderergl2/tr_image_dds.o \
  $(B)/renderergl2/tr_init.o \
  $(B)/renderergl2/tr_light.o \
  $(B)/renderergl2/tr_main.o \
  $(B)/renderergl2/tr_marks.o \
  $(B)/renderergl2/tr_mesh.o \
  $(B)/renderergl2/tr_model.o \
  $(B)/renderergl2/tr_model_iqm.o \
  $(B)/renderergl2/tr_noise.o \
  $(B)/renderergl2/tr_postprocess.o \
  $(B)/renderergl2/tr_scene.o \
  $(B)/renderergl2/tr_shade.o \
  $(B)/renderergl2/tr_shade_calc.o \
  $(B)/renderergl2/tr_shader.o \
  $(B)/renderergl2/tr_shadows.o \
  $(B)/renderergl2/tr_sky.o \
  $(B)/renderergl2/tr_surface.o \
  $(B)/renderergl2/tr_vbo.o \
  $(B)/renderergl2/tr_world.o \
  \
  $(B)/renderercommon/sdl_gamma.o \
  $(B)/renderercommon/sdl_glimp.o

ifneq ($(USE_RENDERER_DLOPEN), 0)
  Q3R2OBJ += \
    $(B)/renderergl1/q_shared.o \
    $(B)/renderergl1/puff.o \
    $(B)/renderergl1/q_math.o \
    $(B)/renderergl1/tr_subs.o
endif

Q3R2STRINGOBJ = \
  $(B)/renderergl2/glsl/bokeh_fp.o \
  $(B)/renderergl2/glsl/bokeh_vp.o \
  $(B)/renderergl2/glsl/calclevels4x_fp.o \
  $(B)/renderergl2/glsl/calclevels4x_vp.o \
  $(B)/renderergl2/glsl/depthblur_fp.o \
  $(B)/renderergl2/glsl/depthblur_vp.o \
  $(B)/renderergl2/glsl/dlight_fp.o \
  $(B)/renderergl2/glsl/dlight_vp.o \
  $(B)/renderergl2/glsl/down4x_fp.o \
  $(B)/renderergl2/glsl/down4x_vp.o \
  $(B)/renderergl2/glsl/fogpass_fp.o \
  $(B)/renderergl2/glsl/fogpass_vp.o \
  $(B)/renderergl2/glsl/generic_fp.o \
  $(B)/renderergl2/glsl/generic_vp.o \
  $(B)/renderergl2/glsl/lightall_fp.o \
  $(B)/renderergl2/glsl/lightall_vp.o \
  $(B)/renderergl2/glsl/pshadow_fp.o \
  $(B)/renderergl2/glsl/pshadow_vp.o \
  $(B)/renderergl2/glsl/shadowfill_fp.o \
  $(B)/renderergl2/glsl/shadowfill_vp.o \
  $(B)/renderergl2/glsl/shadowmask_fp.o \
  $(B)/renderergl2/glsl/shadowmask_vp.o \
  $(B)/renderergl2/glsl/ssao_fp.o \
  $(B)/renderergl2/glsl/ssao_vp.o \
  $(B)/renderergl2/glsl/texturecolor_fp.o \
  $(B)/renderergl2/glsl/texturecolor_vp.o \
  $(B)/renderergl2/glsl/tonemap_fp.o \
  $(B)/renderergl2/glsl/tonemap_vp.o

# GL1

Q3ROBJ = \
  $(B)/renderergl1/tr_animation.o \
  $(B)/renderergl1/tr_backend.o \
  $(B)/renderergl1/tr_bsp.o \
  $(B)/renderergl1/tr_cmds.o \
  $(B)/renderergl1/tr_curve.o \
  $(B)/renderergl1/tr_flares.o \
  $(B)/renderergl1/tr_font.o \
  $(B)/renderergl1/tr_image.o \
  $(B)/renderergl1/tr_image_bmp.o \
  $(B)/renderergl1/tr_image_jpg.o \
  $(B)/renderergl1/tr_image_pcx.o \
  $(B)/renderergl1/tr_image_png.o \
  $(B)/renderergl1/tr_image_tga.o \
  $(B)/renderergl1/tr_init.o \
  $(B)/renderergl1/tr_light.o \
  $(B)/renderergl1/tr_main.o \
  $(B)/renderergl1/tr_marks.o \
  $(B)/renderergl1/tr_mesh.o \
  $(B)/renderergl1/tr_model.o \
  $(B)/renderergl1/tr_model_iqm.o \
  $(B)/renderergl1/tr_noise.o \
  $(B)/renderergl1/tr_scene.o \
  $(B)/renderergl1/tr_shade.o \
  $(B)/renderergl1/tr_shade_calc.o \
  $(B)/renderergl1/tr_shader.o \
  $(B)/renderergl1/tr_shadows.o \
  $(B)/renderergl1/tr_sky.o \
  $(B)/renderergl1/tr_surface.o \
  $(B)/renderergl1/tr_world.o \
  \
  $(B)/renderercommon/sdl_gamma.o \
  $(B)/renderercommon/sdl_glimp.o

ifneq ($(USE_RENDERER_DLOPEN), 0)
  Q3ROBJ += \
    $(B)/renderergl1/q_shared.o \
    $(B)/renderergl1/puff.o \
    $(B)/renderergl1/q_math.o \
    $(B)/renderergl1/tr_subs.o
endif

ifneq ($(USE_INTERNAL_JPEG),0)
  JPGOBJ = \
    $(B)/renderercommon/jaricom.o \
    $(B)/renderercommon/jcapimin.o \
    $(B)/renderercommon/jcapistd.o \
    $(B)/renderercommon/jcarith.o \
    $(B)/renderercommon/jccoefct.o  \
    $(B)/renderercommon/jccolor.o \
    $(B)/renderercommon/jcdctmgr.o \
    $(B)/renderercommon/jchuff.o   \
    $(B)/renderercommon/jcinit.o \
    $(B)/renderercommon/jcmainct.o \
    $(B)/renderercommon/jcmarker.o \
    $(B)/renderercommon/jcmaster.o \
    $(B)/renderercommon/jcomapi.o \
    $(B)/renderercommon/jcparam.o \
    $(B)/renderercommon/jcprepct.o \
    $(B)/renderercommon/jcsample.o \
    $(B)/renderercommon/jctrans.o \
    $(B)/renderercommon/jdapimin.o \
    $(B)/renderercommon/jdapistd.o \
    $(B)/renderercommon/jdarith.o \
    $(B)/renderercommon/jdatadst.o \
    $(B)/renderercommon/jdatasrc.o \
    $(B)/renderercommon/jdcoefct.o \
    $(B)/renderercommon/jdcolor.o \
    $(B)/renderercommon/jddctmgr.o \
    $(B)/renderercommon/jdhuff.o \
    $(B)/renderercommon/jdinput.o \
    $(B)/renderercommon/jdmainct.o \
    $(B)/renderercommon/jdmarker.o \
    $(B)/renderercommon/jdmaster.o \
    $(B)/renderercommon/jdmerge.o \
    $(B)/renderercommon/jdpostct.o \
    $(B)/renderercommon/jdsample.o \
    $(B)/renderercommon/jdtrans.o \
    $(B)/renderercommon/jerror.o \
    $(B)/renderercommon/jfdctflt.o \
    $(B)/renderercommon/jfdctfst.o \
    $(B)/renderercommon/jfdctint.o \
    $(B)/renderercommon/jidctflt.o \
    $(B)/renderercommon/jidctfst.o \
    $(B)/renderercommon/jidctint.o \
    $(B)/renderercommon/jmemmgr.o \
    $(B)/renderercommon/jmemnobs.o \
    $(B)/renderercommon/jquant1.o \
    $(B)/renderercommon/jquant2.o \
    $(B)/renderercommon/jutils.o
endif

ifeq ($(ARCH),x86)
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/snapvector.o
endif
ifeq ($(ARCH),x86_64)
  Q3OBJ += \
    $(B)/client/snapvector.o
endif

ifeq ($(NEED_OPUS),1)
ifeq ($(USE_INTERNAL_OPUS),1)
Q3OBJ += \
  $(B)/client/opus/analysis.o \
  $(B)/client/opus/mlp.o \
  $(B)/client/opus/mlp_data.o \
  $(B)/client/opus/opus.o \
  $(B)/client/opus/opus_decoder.o \
  $(B)/client/opus/opus_encoder.o \
  $(B)/client/opus/opus_multistream.o \
  $(B)/client/opus/opus_multistream_encoder.o \
  $(B)/client/opus/opus_multistream_decoder.o \
  $(B)/client/opus/repacketizer.o \
  \
  $(B)/client/opus/bands.o \
  $(B)/client/opus/celt.o \
  $(B)/client/opus/cwrs.o \
  $(B)/client/opus/entcode.o \
  $(B)/client/opus/entdec.o \
  $(B)/client/opus/entenc.o \
  $(B)/client/opus/kiss_fft.o \
  $(B)/client/opus/laplace.o \
  $(B)/client/opus/mathops.o \
  $(B)/client/opus/mdct.o \
  $(B)/client/opus/modes.o \
  $(B)/client/opus/pitch.o \
  $(B)/client/opus/celt_encoder.o \
  $(B)/client/opus/celt_decoder.o \
  $(B)/client/opus/celt_lpc.o \
  $(B)/client/opus/quant_bands.o \
  $(B)/client/opus/rate.o \
  $(B)/client/opus/vq.o \
  \
  $(B)/client/opus/CNG.o \
  $(B)/client/opus/code_signs.o \
  $(B)/client/opus/init_decoder.o \
  $(B)/client/opus/decode_core.o \
  $(B)/client/opus/decode_frame.o \
  $(B)/client/opus/decode_parameters.o \
  $(B)/client/opus/decode_indices.o \
  $(B)/client/opus/decode_pulses.o \
  $(B)/client/opus/decoder_set_fs.o \
  $(B)/client/opus/dec_API.o \
  $(B)/client/opus/enc_API.o \
  $(B)/client/opus/encode_indices.o \
  $(B)/client/opus/encode_pulses.o \
  $(B)/client/opus/gain_quant.o \
  $(B)/client/opus/interpolate.o \
  $(B)/client/opus/LP_variable_cutoff.o \
  $(B)/client/opus/NLSF_decode.o \
  $(B)/client/opus/NSQ.o \
  $(B)/client/opus/NSQ_del_dec.o \
  $(B)/client/opus/PLC.o \
  $(B)/client/opus/shell_coder.o \
  $(B)/client/opus/tables_gain.o \
  $(B)/client/opus/tables_LTP.o \
  $(B)/client/opus/tables_NLSF_CB_NB_MB.o \
  $(B)/client/opus/tables_NLSF_CB_WB.o \
  $(B)/client/opus/tables_other.o \
  $(B)/client/opus/tables_pitch_lag.o \
  $(B)/client/opus/tables_pulses_per_block.o \
  $(B)/client/opus/VAD.o \
  $(B)/client/opus/control_audio_bandwidth.o \
  $(B)/client/opus/quant_LTP_gains.o \
  $(B)/client/opus/VQ_WMat_EC.o \
  $(B)/client/opus/HP_variable_cutoff.o \
  $(B)/client/opus/NLSF_encode.o \
  $(B)/client/opus/NLSF_VQ.o \
  $(B)/client/opus/NLSF_unpack.o \
  $(B)/client/opus/NLSF_del_dec_quant.o \
  $(B)/client/opus/process_NLSFs.o \
  $(B)/client/opus/stereo_LR_to_MS.o \
  $(B)/client/opus/stereo_MS_to_LR.o \
  $(B)/client/opus/check_control_input.o \
  $(B)/client/opus/control_SNR.o \
  $(B)/client/opus/init_encoder.o \
  $(B)/client/opus/control_codec.o \
  $(B)/client/opus/A2NLSF.o \
  $(B)/client/opus/ana_filt_bank_1.o \
  $(B)/client/opus/biquad_alt.o \
  $(B)/client/opus/bwexpander_32.o \
  $(B)/client/opus/bwexpander.o \
  $(B)/client/opus/debug.o \
  $(B)/client/opus/decode_pitch.o \
  $(B)/client/opus/inner_prod_aligned.o \
  $(B)/client/opus/lin2log.o \
  $(B)/client/opus/log2lin.o \
  $(B)/client/opus/LPC_analysis_filter.o \
  $(B)/client/opus/LPC_inv_pred_gain.o \
  $(B)/client/opus/table_LSF_cos.o \
  $(B)/client/opus/NLSF2A.o \
  $(B)/client/opus/NLSF_stabilize.o \
  $(B)/client/opus/NLSF_VQ_weights_laroia.o \
  $(B)/client/opus/pitch_est_tables.o \
  $(B)/client/opus/resampler.o \
  $(B)/client/opus/resampler_down2_3.o \
  $(B)/client/opus/resampler_down2.o \
  $(B)/client/opus/resampler_private_AR2.o \
  $(B)/client/opus/resampler_private_down_FIR.o \
  $(B)/client/opus/resampler_private_IIR_FIR.o \
  $(B)/client/opus/resampler_private_up2_HQ.o \
  $(B)/client/opus/resampler_rom.o \
  $(B)/client/opus/sigm_Q15.o \
  $(B)/client/opus/sort.o \
  $(B)/client/opus/sum_sqr_shift.o \
  $(B)/client/opus/stereo_decode_pred.o \
  $(B)/client/opus/stereo_encode_pred.o \
  $(B)/client/opus/stereo_find_predictor.o \
  $(B)/client/opus/stereo_quant_pred.o \
  \
  $(B)/client/opus/apply_sine_window_FLP.o \
  $(B)/client/opus/corrMatrix_FLP.o \
  $(B)/client/opus/encode_frame_FLP.o \
  $(B)/client/opus/find_LPC_FLP.o \
  $(B)/client/opus/find_LTP_FLP.o \
  $(B)/client/opus/find_pitch_lags_FLP.o \
  $(B)/client/opus/find_pred_coefs_FLP.o \
  $(B)/client/opus/LPC_analysis_filter_FLP.o \
  $(B)/client/opus/LTP_analysis_filter_FLP.o \
  $(B)/client/opus/LTP_scale_ctrl_FLP.o \
  $(B)/client/opus/noise_shape_analysis_FLP.o \
  $(B)/client/opus/prefilter_FLP.o \
  $(B)/client/opus/process_gains_FLP.o \
  $(B)/client/opus/regularize_correlations_FLP.o \
  $(B)/client/opus/residual_energy_FLP.o \
  $(B)/client/opus/solve_LS_FLP.o \
  $(B)/client/opus/warped_autocorrelation_FLP.o \
  $(B)/client/opus/wrappers_FLP.o \
  $(B)/client/opus/autocorrelation_FLP.o \
  $(B)/client/opus/burg_modified_FLP.o \
  $(B)/client/opus/bwexpander_FLP.o \
  $(B)/client/opus/energy_FLP.o \
  $(B)/client/opus/inner_product_FLP.o \
  $(B)/client/opus/k2a_FLP.o \
  $(B)/client/opus/levinsondurbin_FLP.o \
  $(B)/client/opus/LPC_inv_pred_gain_FLP.o \
  $(B)/client/opus/pitch_analysis_core_FLP.o \
  $(B)/client/opus/scale_copy_vector_FLP.o \
  $(B)/client/opus/scale_vector_FLP.o \
  $(B)/client/opus/schur_FLP.o \
  $(B)/client/opus/sort_FLP.o \
  \
  $(B)/client/http.o \
  $(B)/client/info.o \
  $(B)/client/internal.o \
  $(B)/client/opusfile.o \
  $(B)/client/stream.o \
  $(B)/client/wincerts.o
endif
endif

ifeq ($(NEED_OGG),1)
ifeq ($(USE_INTERNAL_OGG),1)
Q3OBJ += \
  $(B)/client/bitwise.o \
  $(B)/client/framing.o
endif
endif

ifeq ($(USE_CODEC_VORBIS),1)
ifeq ($(USE_INTERNAL_VORBIS),1)
Q3OBJ += \
  $(B)/client/vorbis/analysis.o \
  $(B)/client/vorbis/bitrate.o \
  $(B)/client/vorbis/block.o \
  $(B)/client/vorbis/codebook.o \
  $(B)/client/vorbis/envelope.o \
  $(B)/client/vorbis/floor0.o \
  $(B)/client/vorbis/floor1.o \
  $(B)/client/vorbis/info.o \
  $(B)/client/vorbis/lookup.o \
  $(B)/client/vorbis/lpc.o \
  $(B)/client/vorbis/lsp.o \
  $(B)/client/vorbis/mapping0.o \
  $(B)/client/vorbis/mdct.o \
  $(B)/client/vorbis/psy.o \
  $(B)/client/vorbis/registry.o \
  $(B)/client/vorbis/res0.o \
  $(B)/client/vorbis/smallft.o \
  $(B)/client/vorbis/sharedbook.o \
  $(B)/client/vorbis/synthesis.o \
  $(B)/client/vorbis/vorbisfile.o \
  $(B)/client/vorbis/window.o
endif
endif

ifeq ($(USE_INTERNAL_ZLIB),1)
Q3OBJ += \
  $(B)/client/adler32.o \
  $(B)/client/crc32.o \
  $(B)/client/inffast.o \
  $(B)/client/inflate.o \
  $(B)/client/inftrees.o \
  $(B)/client/zutil.o
endif

#-bbq
ifeq ($(USE_RESTCLIENT),1)
  Q3OBJ += \
  $(B)/client/restclient/connection.o \
  $(B)/client/restclient/helpers.o \
  $(B)/client/restclient/restclient.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifneq ($(findstring $(ARCH),x86 x86_64),)
    Q3OBJ += $(B)/client/vm_x86.o
  endif
  ifneq ($(findstring $(ARCH),ppc ppc64),)
    Q3OBJ += $(B)/client/vm_powerpc.o $(B)/client/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),sparc)
    Q3OBJ += $(B)/client/vm_sparc.o
  endif
endif

ifdef MINGW
  Q3OBJ += \
    $(B)/client/win_resource.o \
    $(B)/client/sys_win32.o \
    $(B)/client/sys_win32_default_homepath.o
else
  Q3OBJ += \
    $(B)/client/sys_unix.o
endif

ifeq ($(PLATFORM),darwin)
  Q3OBJ += \
    $(B)/client/sys_osx.o
endif

ifeq ($(USE_MUMBLE),1)
  Q3OBJ += \
    $(B)/client/libmumblelink.o
endif

ifneq ($(USE_RENDERER_DLOPEN),0)
$(B)/$(CLIENTBIN)$(FULLBINEXT): $(Q3OBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -std=c++1y $(CXXFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) $(Q3OBJ) \
		$(LIBSDLMAIN) $(CLIENT_LIBS) $(LIBS) -o $@ 

$(B)/renderer_opengl1$(SHLIBNAME): $(Q3ROBJ) $(JPGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(SHLIBLDFLAGS) -o $@ $(Q3ROBJ) $(JPGOBJ) \
		$(THREAD_LIBS) $(LIBSDLMAIN) $(RENDERER_LIBS) $(LDFLAGS)

$(B)/renderer_opengl2$(SHLIBNAME): $(Q3R2OBJ) $(Q3R2STRINGOBJ) $(JPGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(SHLIBLDFLAGS) -o $@ $(Q3R2OBJ) $(Q3R2STRINGOBJ) $(JPGOBJ) \
		$(THREAD_LIBS) $(LIBSDLMAIN) $(RENDERER_LIBS) $(LDFLAGS)
else
$(B)/$(CLIENTBIN)$(FULLBINEXT): $(Q3OBJ) $(Q3ROBJ) $(JPGOBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -std=c++1y $(CXXFLAGS) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) \
		-o $@ $(Q3OBJ) $(Q3ROBJ) $(JPGOBJ) \
		$(LIBSDLMAIN) $(CLIENT_LIBS) $(RENDERER_LIBS) $(LIBS)

$(B)/$(CLIENTBIN)_opengl2$(FULLBINEXT): $(Q3OBJ) $(Q3R2OBJ) $(Q3R2STRINGOBJ) $(JPGOBJ) $(LIBSDLMAIN)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) -std=c++1y $(CXXFLAGS) $(CLIENT_CFLAGS) $(CFLAGS) $(CLIENT_LDFLAGS) $(LDFLAGS) \
		-o $@ $(Q3OBJ) $(Q3R2OBJ) $(Q3R2STRINGOBJ) $(JPGOBJ) \
		$(LIBSDLMAIN) $(CLIENT_LIBS) $(RENDERER_LIBS) $(LIBS)
endif

ifneq ($(strip $(LIBSDLMAIN)),)
ifneq ($(strip $(LIBSDLMAINSRC)),)
$(LIBSDLMAIN) : $(LIBSDLMAINSRC)
	cp $< $@
	$(RANLIB) $@
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
  $(B)/ded/q3_lauxlib.o \
  \
  $(B)/ded/cm_load.o \
  $(B)/ded/cm_patch.o \
  $(B)/ded/cm_polylib.o \
  $(B)/ded/cm_test.o \
  $(B)/ded/cm_trace.o \
  $(B)/ded/cmd.o \
  $(B)/ded/common.o \
  $(B)/ded/crypto.o \
  $(B)/ded/cvar.o \
  $(B)/ded/files.o \
  $(B)/ded/md4.o \
  $(B)/ded/msg.o \
  $(B)/ded/net_chan.o \
  $(B)/ded/net_ip.o \
  $(B)/ded/huffman.o \
  $(B)/ded/parse.o \
  \
  $(B)/ded/q_math.o \
  $(B)/ded/q_shared.o \
  \
  $(B)/ded/unzip.o \
  $(B)/ded/ioapi.o \
  $(B)/ded/vm.o \
  $(B)/ded/vm_interpreted.o \
  \
  $(B)/ded/null_client.o \
  $(B)/ded/null_input.o \
  $(B)/ded/null_snddma.o \
  \
  $(B)/ded/con_log.o \
  $(B)/ded/sys_main.o

ifeq ($(ARCH),x86)
  Q3DOBJ += \
      $(B)/ded/matha.o \
      $(B)/ded/snapvector.o
endif
ifeq ($(ARCH),x86_64)
  Q3DOBJ += \
      $(B)/ded/snapvector.o
endif

Q3DOBJ += $(LUAOBJ) $(SCRIPTOBJ) $(NETTLEOBJ)

ifeq ($(USE_INTERNAL_ZLIB),1)
Q3DOBJ += \
  $(B)/ded/adler32.o \
  $(B)/ded/crc32.o \
  $(B)/ded/inffast.o \
  $(B)/ded/inflate.o \
  $(B)/ded/inftrees.o \
  $(B)/ded/zutil.o
endif

ifeq ($(HAVE_VM_COMPILED),true)
  ifneq ($(findstring $(ARCH),x86 x86_64),)
    Q3DOBJ += \
      $(B)/ded/vm_x86.o
  endif
  ifneq ($(findstring $(ARCH),ppc ppc64),)
    Q3DOBJ += $(B)/ded/vm_powerpc.o $(B)/ded/vm_powerpc_asm.o
  endif
  ifeq ($(ARCH),sparc)
    Q3DOBJ += $(B)/ded/vm_sparc.o
  endif
endif

ifdef MINGW
  Q3DOBJ += \
    $(B)/ded/win_resource.o \
    $(B)/ded/sys_win32.o \
    $(B)/ded/sys_win32_default_homepath.o \
    $(B)/ded/con_win32.o
else
  Q3DOBJ += \
    $(B)/ded/sys_unix.o \
    $(B)/ded/con_tty.o
endif

ifeq ($(PLATFORM),darwin)
  Q3DOBJ += \
    $(B)/ded/sys_osx.o
endif

$(B)/$(SERVERBIN)$(FULLBINEXT): $(Q3DOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(Q3DOBJ) $(LIBS)

#############################################################################
## TREMULOUS CGAME
#############################################################################

CGOBJ_ = \
  $(B)/$(BASEGAME)/cgame/cg_main.o \
  $(B)/$(BASEGAME)/cgame/bg_misc.o \
  $(B)/$(BASEGAME)/cgame/bg_pmove.o \
  $(B)/$(BASEGAME)/cgame/bg_slidemove.o \
  $(B)/$(BASEGAME)/cgame/bg_lib.o \
  $(B)/$(BASEGAME)/cgame/bg_alloc.o \
  $(B)/$(BASEGAME)/cgame/bg_voice.o \
  $(B)/$(BASEGAME)/cgame/cg_consolecmds.o \
  $(B)/$(BASEGAME)/cgame/cg_buildable.o \
  $(B)/$(BASEGAME)/cgame/cg_animation.o \
  $(B)/$(BASEGAME)/cgame/cg_animmapobj.o \
  $(B)/$(BASEGAME)/cgame/cg_draw.o \
  $(B)/$(BASEGAME)/cgame/cg_drawtools.o \
  $(B)/$(BASEGAME)/cgame/cg_ents.o \
  $(B)/$(BASEGAME)/cgame/cg_event.o \
  $(B)/$(BASEGAME)/cgame/cg_marks.o \
  $(B)/$(BASEGAME)/cgame/cg_players.o \
  $(B)/$(BASEGAME)/cgame/cg_playerstate.o \
  $(B)/$(BASEGAME)/cgame/cg_predict.o \
  $(B)/$(BASEGAME)/cgame/cg_servercmds.o \
  $(B)/$(BASEGAME)/cgame/cg_snapshot.o \
  $(B)/$(BASEGAME)/cgame/cg_view.o \
  $(B)/$(BASEGAME)/cgame/cg_weapons.o \
  $(B)/$(BASEGAME)/cgame/cg_scanner.o \
  $(B)/$(BASEGAME)/cgame/cg_attachment.o \
  $(B)/$(BASEGAME)/cgame/cg_trails.o \
  $(B)/$(BASEGAME)/cgame/cg_particles.o \
  $(B)/$(BASEGAME)/cgame/cg_tutorial.o \
  $(B)/$(BASEGAME)/cgame/cg_rangemarker.o \
  $(B)/$(BASEGAME)/cgame/ui_shared.o \
  \
  $(B)/$(BASEGAME)/qcommon/q_math.o \
  $(B)/$(BASEGAME)/qcommon/q_shared.o

CGOBJ = $(CGOBJ_) $(B)/$(BASEGAME)/cgame/cg_syscalls.o
CGVMOBJ = $(CGOBJ_:%.o=%.asm)

$(B)/$(BASEGAME)/cgame$(SHLIBNAME): $(CGOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(SHLIBLDFLAGS) $(LDFLAGS) -o $@ $(CGOBJ)

$(B)/$(BASEGAME)/vm/cgame.qvm: $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm

#############################################################################
## TREMULOUS CGAME (1.1 COMPATIBLE)
#############################################################################

CGOBJ11_ = \
  $(B)/$(BASEGAME)/11/cgame/cg_main.o \
  $(B)/$(BASEGAME)/cgame/bg_misc.o \
  $(B)/$(BASEGAME)/cgame/bg_pmove.o \
  $(B)/$(BASEGAME)/cgame/bg_slidemove.o \
  $(B)/$(BASEGAME)/cgame/bg_lib.o \
  $(B)/$(BASEGAME)/cgame/bg_alloc.o \
  $(B)/$(BASEGAME)/cgame/bg_voice.o \
  $(B)/$(BASEGAME)/11/cgame/cg_consolecmds.o \
  $(B)/$(BASEGAME)/cgame/cg_buildable.o \
  $(B)/$(BASEGAME)/cgame/cg_animation.o \
  $(B)/$(BASEGAME)/cgame/cg_animmapobj.o \
  $(B)/$(BASEGAME)/cgame/cg_draw.o \
  $(B)/$(BASEGAME)/cgame/cg_drawtools.o \
  $(B)/$(BASEGAME)/cgame/cg_ents.o \
  $(B)/$(BASEGAME)/cgame/cg_event.o \
  $(B)/$(BASEGAME)/cgame/cg_marks.o \
  $(B)/$(BASEGAME)/cgame/cg_players.o \
  $(B)/$(BASEGAME)/cgame/cg_playerstate.o \
  $(B)/$(BASEGAME)/cgame/cg_predict.o \
  $(B)/$(BASEGAME)/11/cgame/cg_servercmds.o \
  $(B)/$(BASEGAME)/11/cgame/cg_snapshot.o \
  $(B)/$(BASEGAME)/cgame/cg_view.o \
  $(B)/$(BASEGAME)/cgame/cg_weapons.o \
  $(B)/$(BASEGAME)/cgame/cg_scanner.o \
  $(B)/$(BASEGAME)/cgame/cg_attachment.o \
  $(B)/$(BASEGAME)/cgame/cg_trails.o \
  $(B)/$(BASEGAME)/cgame/cg_particles.o \
  $(B)/$(BASEGAME)/cgame/cg_tutorial.o \
  $(B)/$(BASEGAME)/cgame/cg_rangemarker.o \
  $(B)/$(BASEGAME)/cgame/ui_shared.o \
  \
  $(B)/$(BASEGAME)/qcommon/q_math.o \
  $(B)/$(BASEGAME)/qcommon/q_shared.o

CGVMOBJ11 = $(CGOBJ11_:%.o=%.asm)

$(B)/$(BASEGAME)_11/vm/cgame.qvm: $(CGVMOBJ11) $(CGDIR)/cg_syscalls_11.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM_11 $@"
	$(Q)$(Q3ASM) -o $@ $(CGVMOBJ11) $(CGDIR)/cg_syscalls_11.asm

#############################################################################
## TREMULOUS GAME
#############################################################################

GOBJ_ = \
  $(B)/$(BASEGAME)/game/g_main.o \
  $(B)/$(BASEGAME)/game/bg_misc.o \
  $(B)/$(BASEGAME)/game/bg_pmove.o \
  $(B)/$(BASEGAME)/game/bg_slidemove.o \
  $(B)/$(BASEGAME)/game/bg_lib.o \
  $(B)/$(BASEGAME)/game/bg_alloc.o \
  $(B)/$(BASEGAME)/game/bg_voice.o \
  $(B)/$(BASEGAME)/game/g_active.o \
  $(B)/$(BASEGAME)/game/g_client.o \
  $(B)/$(BASEGAME)/game/g_cmds.o \
  $(B)/$(BASEGAME)/game/g_combat.o \
  $(B)/$(BASEGAME)/game/g_physics.o \
  $(B)/$(BASEGAME)/game/g_buildable.o \
  $(B)/$(BASEGAME)/game/g_misc.o \
  $(B)/$(BASEGAME)/game/g_missile.o \
  $(B)/$(BASEGAME)/game/g_mover.o \
  $(B)/$(BASEGAME)/game/g_session.o \
  $(B)/$(BASEGAME)/game/g_spawn.o \
  $(B)/$(BASEGAME)/game/g_svcmds.o \
  $(B)/$(BASEGAME)/game/g_target.o \
  $(B)/$(BASEGAME)/game/g_team.o \
  $(B)/$(BASEGAME)/game/g_trigger.o \
  $(B)/$(BASEGAME)/game/g_utils.o \
  $(B)/$(BASEGAME)/game/g_maprotation.o \
  $(B)/$(BASEGAME)/game/g_playermodel.o \
  $(B)/$(BASEGAME)/game/g_weapon.o \
  $(B)/$(BASEGAME)/game/g_weapondrop.o \
  $(B)/$(BASEGAME)/game/g_admin.o \
  $(B)/$(BASEGAME)/game/g_namelog.o \
  \
  $(B)/$(BASEGAME)/qcommon/q_math.o \
  $(B)/$(BASEGAME)/qcommon/q_shared.o

GOBJ = $(GOBJ_) $(B)/$(BASEGAME)/game/g_syscalls.o
GVMOBJ = $(GOBJ_:%.o=%.asm)

$(B)/$(BASEGAME)/game$(SHLIBNAME): $(GOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) $(SHLIBLDFLAGS) $(LDFLAGS) -o $@ $(GOBJ)

$(B)/$(BASEGAME)/vm/game.qvm: $(GVMOBJ) $(GDIR)/g_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(GVMOBJ) $(GDIR)/g_syscalls.asm



#############################################################################
## TREMULOUS UI
#############################################################################

UIOBJ_ = \
  $(B)/$(BASEGAME)/ui/ui_main.o \
  $(B)/$(BASEGAME)/ui/ui_atoms.o \
  $(B)/$(BASEGAME)/ui/ui_shared.o \
  $(B)/$(BASEGAME)/ui/ui_gameinfo.o \
  \
  $(B)/$(BASEGAME)/ui/bg_alloc.o \
  $(B)/$(BASEGAME)/ui/bg_voice.o \
  $(B)/$(BASEGAME)/ui/bg_misc.o \
  $(B)/$(BASEGAME)/ui/bg_lib.o \
  $(B)/$(BASEGAME)/qcommon/q_math.o \
  $(B)/$(BASEGAME)/qcommon/q_shared.o

UIOBJ = $(UIOBJ_) $(B)/$(BASEGAME)/ui/ui_syscalls.o
UIVMOBJ = $(UIOBJ_:%.o=%.asm)

$(B)/$(BASEGAME)/ui$(SHLIBNAME): $(UIOBJ)
	$(echo_cmd) "LD $@"
	$(Q)$(CC) -I${ASSETS_DIR}/ui $(SHLIBLDFLAGS) $(LDFLAGS) -o $@ $(UIOBJ)

$(B)/$(BASEGAME)/vm/ui.qvm: $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm

#############################################################################
## TREMULOUS UI (1.1 compatibility)
#############################################################################

UIOBJ11_ = \
  $(B)/$(BASEGAME)/11/ui/ui_main.o \
  $(B)/$(BASEGAME)/ui/ui_atoms.o \
  $(B)/$(BASEGAME)/ui/ui_shared.o \
  $(B)/$(BASEGAME)/ui/ui_gameinfo.o \
  \
  $(B)/$(BASEGAME)/ui/bg_alloc.o \
  $(B)/$(BASEGAME)/ui/bg_voice.o \
  $(B)/$(BASEGAME)/ui/bg_misc.o \
  $(B)/$(BASEGAME)/ui/bg_lib.o \
  $(B)/$(BASEGAME)/qcommon/q_math.o \
  $(B)/$(BASEGAME)/qcommon/q_shared.o
UIVMOBJ11 = $(UIOBJ11_:%.o=%.asm)

# XXX no dynamic library?

$(B)/$(BASEGAME)_11/vm/ui.qvm: $(UIVMOBJ11) $(UIDIR)/ui_syscalls_11.asm $(Q3ASM)
	$(echo_cmd) "Q3ASM $@"
	$(Q)$(Q3ASM) -o $@ $(UIVMOBJ11) $(UIDIR)/ui_syscalls_11.asm

#############################################################################
## QVM Package
#############################################################################

$(B)/$(BASEGAME)/vms-gpp-$(VERSION).pk3: $(B)/$(BASEGAME)/vm/ui.qvm $(B)/$(BASEGAME)/vm/cgame.qvm $(B)/$(BASEGAME)/vm/game.qvm
	@(cd $(B)/$(BASEGAME) && zip -r vms-$(VERSION).pk3 vm/)

$(B)/$(BASEGAME)_11/vms-1.1.0-$(VERSION).pk3: $(B)/$(BASEGAME)_11/vm/ui.qvm $(B)/$(BASEGAME)_11/vm/cgame.qvm 
	@(cd $(B)/$(BASEGAME)_11 && zip -r vms-$(VERSION).pk3 vm/)


#############################################################################
## Assets Package
#############################################################################

$(B)/$(BASEGAME)/data-$(VERSION).pk3: $(ASSETS_DIR)/ui/main.menu
	@(cd $(ASSETS_DIR) && zip -r data-$(VERSION).pk3 *)
	@mv $(ASSETS_DIR)/data-$(VERSION).pk3 $(B)/$(BASEGAME)

#############################################################################
## CLIENT/SERVER RULES
#############################################################################

$(B)/client/%.o: $(ASMDIR)/%.s
	$(DO_AS)

# k8 so inline assembler knows about SSE
$(B)/client/%.o: $(ASMDIR)/%.c
	$(DO_CC) -march=k8

$(B)/client/%.o: $(CDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(CMDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(CDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(SDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(CMDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(OGGDIR)/src/%.c
	$(DO_CC)

$(B)/client/vorbis/%.o: $(VORBISDIR)/lib/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/src/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/celt/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/silk/%.c
	$(DO_CC)

$(B)/client/opus/%.o: $(OPUSDIR)/silk/float/%.c
	$(DO_CC)

$(B)/client/%.o: $(OPUSFILEDIR)/src/%.c
	$(DO_CC)

$(B)/client/%.o: $(ZDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDLDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SDLDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(SYSDIR)/%.c
	$(DO_CC)

$(B)/client/%.o: $(SYSDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(SYSDIR)/%.mm
	$(DO_CXX)

#-wtf
$(B)/client/restclient/%.o: $(RESTDIR)/%.cpp
	$(DO_CXX)

$(B)/client/%.o: $(SYSDIR)/%.rc
	$(DO_WINDRES)

### GL1

$(B)/renderercommon/%.o: $(SDLDIR)/%.c
	$(DO_RENDERER_COMMON_CC)
$(B)/renderercommon/%.o: $(SDLDIR)/%.cpp
	$(DO_RENDERER_COMMON_CXX)
$(B)/renderercommon/%.o: $(JPDIR)/%.c
	$(DO_RENDERER_COMMON_CC)

$(B)/renderergl1/%.o: $(RCOMMONDIR)/%.c
	$(DO_RENDERER_COMMON_CC)
$(B)/renderergl1/%.o: $(RCOMMONDIR)/%.cpp
	$(DO_RENDERER_COMMON_CXX)
$(B)/renderergl1/%.o: $(RGL1DIR)/%.c
	$(DO_RENDERERGL1_CC)
$(B)/renderergl1/%.o: $(RGL1DIR)/%.cpp
	$(DO_RENDERERGL1_CXX)
$(B)/renderergl1/%.o: $(CMDIR)/%.c
	$(DO_RENDERERGL1_CC)
$(B)/renderergl1/%.o: $(CMDIR)/%.cpp
	$(DO_RENDERERGL1_CXX)

### GL2

$(B)/renderergl2/glsl/%.c: $(RGL2DIR)/glsl/%.glsl
	$(DO_REF_STR)
$(B)/renderergl2/glsl/%.o: $(B)/renderergl2/glsl/%.c
	$(DO_RENDERERGL2_CC)
$(B)/renderergl2/%.o: $(RCOMMONDIR)/%.c
	$(DO_RENDERER_COMMON_CC)
$(B)/renderergl2/%.o: $(RGL2DIR)/%.c
	$(DO_RENDERERGL2_CC)
$(B)/renderergl2/%.o: $(RCOMMONDIR)/%.cpp
	$(DO_RENDERER_COMMON_CXX)
$(B)/renderergl2/%.o: $(RGL2DIR)/%.cpp
	$(DO_RENDERERGL2_CXX)

$(B)/ded/%.o: $(ASMDIR)/%.s
	$(DO_DED_AS)

# k8 so inline assembler knows about SSE
$(B)/ded/%.o: $(ASMDIR)/%.c
	$(DO_DED_CC) -march=k8

$(B)/ded/%.o: $(SDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(SDIR)/%.cpp
	$(DO_DED_CXX)

$(B)/ded/%.o: $(CMDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(CMDIR)/%.cpp
	$(DO_DED_CXX)

$(B)/ded/%.o: $(ZDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(SYSDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(SYSDIR)/%.cpp
	$(DO_DED_CXX)

$(B)/ded/%.o: $(SYSDIR)/%.mm
	$(DO_DED_CXX)

$(B)/ded/%.o: $(SYSDIR)/%.rc
	$(DO_WINDRES)

$(B)/ded/%.o: $(NDIR)/%.c
	$(DO_DED_CC)

$(B)/ded/%.o: $(NDIR)/%.cpp
	$(DO_DED_CXX)

# Extra dependencies to ensure the git version is incorporated
ifeq ($(USE_GIT),1)
  $(B)/client/cl_console.o : .git/index
  $(B)/client/common.o : .git/index
  $(B)/ded/common.o : .git/index
endif


#############################################################################
## GAME MODULE RULES
#############################################################################

# CGAME
$(B)/$(BASEGAME)/cgame/bg_%.o: $(GDIR)/bg_%.c
	$(DO_CGAME_CC)

$(B)/$(BASEGAME)/cgame/ui_%.o: $(UIDIR)/ui_%.c
	$(DO_CGAME_CC)

$(B)/$(BASEGAME)/cgame/%.o: $(CGDIR)/%.c
	$(DO_CGAME_CC)

$(B)/$(BASEGAME)/cgame/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

$(B)/$(BASEGAME)/cgame/ui_%.asm: $(UIDIR)/ui_%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

$(B)/$(BASEGAME)/cgame/%.asm: $(CGDIR)/%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC)

# CGAME (1.1 COMPATIBLE)
#$(B)/$(BASEGAME)_11/cgame/bg_%.o: $(GDIR)/bg_%.c
#	$(DO_CGAME_CC_11)
#
#$(B)/$(BASEGAME)_11/cgame/ui_%.o: $(UIDIR)/ui_%.c
#	$(DO_CGAME_CC_11)
#
#$(B)/$(BASEGAME)_11/cgame/%.o: $(CGDIR)/%.c
#	$(DO_CGAME_CC_11)

$(B)/$(BASEGAME)/11/cgame/%.asm: $(CGDIR)/%.c $(Q3LCC)
	$(DO_CGAME_Q3LCC_11)

# GAME
$(B)/$(BASEGAME)/game/%.o: $(GDIR)/%.c
	$(DO_GAME_CC)

$(B)/$(BASEGAME)/game/%.asm: $(GDIR)/%.c $(Q3LCC)
	$(DO_GAME_Q3LCC)

# UI
$(B)/$(BASEGAME)/ui/bg_%.o: $(GDIR)/bg_%.c
	$(DO_UI_CC)

$(B)/$(BASEGAME)/ui/%.o: $(UIDIR)/%.c
	$(DO_UI_CC)

$(B)/$(BASEGAME)/ui/bg_%.asm: $(GDIR)/bg_%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

$(B)/$(BASEGAME)/ui/%.asm: $(UIDIR)/%.c $(Q3LCC)
	$(DO_UI_Q3LCC)

# UI (1.1 COMPATIBLE)
$(B)/$(BASEGAME)/11/ui/%.asm: $(UIDIR)/%.c $(Q3LCC)
	$(DO_UI_Q3LCC_11)

$(B)/$(BASEGAME)/qcommon/%.o: $(CMDIR)/%.c
	$(DO_SHLIB_CC)

$(B)/$(BASEGAME)/qcommon/%.asm: $(CMDIR)/%.c $(Q3LCC)
	$(DO_Q3LCC)


#############################################################################
# MISC
#############################################################################

OBJ = $(Q3OBJ) $(Q3ROBJ) $(Q3R2OBJ) $(Q3DOBJ) $(JPGOBJ) \
  $(GOBJ) $(CGOBJ) $(UIOBJ) $(LUAOBJ) $(SCRIPTOBJ) $(NETTLEOBJ) \
  $(GVMOBJ) $(CGVMOBJ) $(UIVMOBJ) $(GRANGEROBJ)
TOOLSOBJ = $(LBURGOBJ) $(Q3CPPOBJ) $(Q3RCCOBJ) $(Q3LCCOBJ) $(Q3ASMOBJ)
STRINGOBJ = $(Q3R2STRINGOBJ)

clean: clean-debug clean-release
	@rm -f $(B)/compile_commands.json $(B)/compile_commands.txt $(B)/compile_commands.txt-e

clean-debug:
	@$(MAKE) clean2 B=$(BD)

clean-release:
	@$(MAKE) clean2 B=$(BR)

clean2:
	@echo "CLEAN $(B)"
	@rm -rf $(B)/scripts
	@rm -f $(OBJ)
	@rm -f $(OBJ_D_FILES)
	@rm -f $(STRINGOBJ)
	@rm -f $(TARGETS)

toolsclean: toolsclean-debug toolsclean-release

toolsclean-debug:
	@$(MAKE) toolsclean2 B=$(BD)

toolsclean-release:
	@$(MAKE) toolsclean2 B=$(BR)

toolsclean2:
	@echo "TOOLS_CLEAN $(B)"
	@rm -f $(TOOLSOBJ)
	@rm -f $(TOOLSOBJ_D_FILES)
	@rm -f $(LBURG) $(DAGCHECK_C) $(Q3RCC) $(Q3CPP) $(Q3LCC) $(Q3ASM)

distclean: clean toolsclean
	@rm -rf $(BUILD_DIR)

dist:
	git archive --format zip --output $(CLIENTBIN)-$(VERSION).zip HEAD

#############################################################################
# DEPENDENCIES
#############################################################################

ifneq ($(B),)
  OBJ_D_FILES=$(filter %.d,$(OBJ:%.o=%.d))
  TOOLSOBJ_D_FILES=$(filter %.d,$(TOOLSOBJ:%.o=%.d))
  -include $(OBJ_D_FILES) $(TOOLSOBJ_D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release \
	debug default dist distclean makedirs release targets \
	toolsclean toolsclean2 toolsclean-debug toolsclean-release \
	$(OBJ_D_FILES) $(TOOLSOBJ_D_FILES) $(B)/scripts \
	$(B)/$(BASEGAME)/data-$(VERSION).pk3 \
	$(B)/$(BASEGAME)_11/vms-$(VERSION).pk3 \
	$(B)/$(BASEGAME)/vms-$(VERSION).pk3 \
	$(B).zip

# If the target name contains "clean", don't do a parallel build
ifneq ($(findstring clean, $(MAKECMDGOALS)),)
.NOTPARALLEL:
endif
