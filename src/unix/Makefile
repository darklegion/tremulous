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

COMPILE_PLATFORM=$(shell uname|sed -e s/_.*//|tr A-Z a-z)

ifeq ($(COMPILE_PLATFORM),darwin)
  # Apple does some things a little differently...
  COMPILE_ARCH=$(shell uname -p | sed -e s/i.86/i386/)
else
  COMPILE_ARCH=$(shell uname -m | sed -e s/i.86/i386/)
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

# Where we are building from (where the source code should be!)
ifndef MOUNT_DIR
MOUNT_DIR=..
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

ifndef USE_LOCAL_HEADERS
USE_LOCAL_HEADERS=1
endif

#############################################################################

BD=debug-$(PLATFORM)-$(ARCH)
BR=release-$(PLATFORM)-$(ARCH)
CDIR=$(MOUNT_DIR)/client
SDIR=$(MOUNT_DIR)/server
RDIR=$(MOUNT_DIR)/renderer
CMDIR=$(MOUNT_DIR)/qcommon
UDIR=$(MOUNT_DIR)/unix
W32DIR=$(MOUNT_DIR)/win32
GDIR=$(MOUNT_DIR)/game
CGDIR=$(MOUNT_DIR)/cgame
BLIBDIR=$(MOUNT_DIR)/botlib
NDIR=$(MOUNT_DIR)/null
UIDIR=$(MOUNT_DIR)/ui
JPDIR=$(MOUNT_DIR)/jpeg-6

# extract version info
VERSION=$(shell grep Q3_VERSION ../qcommon/q_shared.h | \
  sed -e 's/.*Q3\ \(.*\)"/\1/')

#############################################################################
# SETUP AND BUILD -- LINUX
#############################################################################

## Defaults
VM_PPC=

LIB=lib

INSTALL=install
MKDIR=mkdir

ifeq ($(PLATFORM),linux)

  CC=gcc

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

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_VIDEO=1 -DUSE_SDL_SOUND=1 $(shell sdl-config --cflags)
    GL_CFLAGS =
  else
    GL_CFLAGS = -I/usr/X11R6/include
  endif

  OPTIMIZE = -O3 -ffast-math -funroll-loops -fomit-frame-pointer

  ifeq ($(ARCH),x86_64)
    OPTIMIZE = -O3 -fomit-frame-pointer -ffast-math -funroll-loops \
      -falign-loops=2 -falign-jumps=2 -falign-functions=2 \
      -fstrength-reduce
    # experimental x86_64 jit compiler! you need GNU as
    HAVE_VM_COMPILED = true
  else
  ifeq ($(ARCH),i386)
    OPTIMIZE = -O3 -march=i686 -fomit-frame-pointer -ffast-math \
      -funroll-loops -falign-loops=2 -falign-jumps=2 \
      -falign-functions=2 -fstrength-reduce
    HAVE_VM_COMPILED=true
  else
  ifeq ($(ARCH),ppc)
    BASE_CFLAGS += -maltivec
    ifneq ($(VM_PPC),)
      HAVE_VM_COMPILED=true
    endif
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

  ifeq ($(ARCH),i386)
    # linux32 make ...
    BASE_CFLAGS += -m32
    LDFLAGS+=-m32
  endif

else # ifeq Linux

#############################################################################
# SETUP AND BUILD -- MAC OS X
#############################################################################

ifeq ($(PLATFORM),darwin)
  CC=gcc

  # !!! FIXME: calling conventions are still broken! See Bugzilla #2519
  #VM_PPC=vm_ppc_new

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes
  BASE_CFLAGS += -DMACOS_X -fno-common -pipe

  # Always include debug symbols...you can strip the binary later...
  BASE_CFLAGS += -gfull

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1
    ifeq ($(USE_OPENAL_DLOPEN),1)
      BASE_CFLAGS += -DUSE_OPENAL_DLOPEN=1
    endif
  endif

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_VIDEO=1 -DUSE_SDL_SOUND=1 -D_THREAD_SAFE=1 -I../SDL12/include
    GL_CFLAGS =
  endif

  OPTIMIZE = -O3 -ffast-math -falign-loops=16

  ifeq ($(ARCH),ppc)
  BASE_CFLAGS += -faltivec
    ifneq ($(VM_PPC),)
      HAVE_VM_COMPILED=true
    endif
  endif

  ifeq ($(ARCH),i386)
    # !!! FIXME: x86-specific flags here...
  endif

  ifneq ($(HAVE_VM_COMPILED),true)
    BASE_CFLAGS += -DNO_VM_COMPILED
  endif

  DEBUG_CFLAGS = $(BASE_CFLAGS) -g -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=dylib
  SHLIBCFLAGS=-fPIC -fno-common
  SHLIBLDFLAGS=-dynamiclib $(LDFLAGS)

  NOTSHLIBCFLAGS=-mdynamic-no-pic

  #THREAD_LDFLAGS=-lpthread
  #LDFLAGS=-ldl -lm
  LDFLAGS += -framework Carbon

  ifeq ($(USE_SDL),1)
    # We copy sdlmain before ranlib'ing it so that subversion doesn't think
    #  the file has been modified by each build.
    LIBSDLMAIN=$(B)/libSDLmain.a
    LIBSDLMAINSRC=../libs/macosx/libSDLmain.a
    CLIENT_LDFLAGS=-framework Cocoa -framework OpenGL ../libs/macosx/libSDL-1.2.0.dylib
  else
    # !!! FIXME: frameworks: OpenGL, Carbon, etc...
    #CLIENT_LDFLAGS=-L/usr/X11R6/$(LIB) -lX11 -lXext -lXxf86dga -lXxf86vm
  endif

  # -framework OpenAL requires 10.4 or later...for builds shipping to the
  #  public, you'll want to use USE_OPENAL_DLOPEN and ship your own OpenAL
  #  library (http://openal.org/ or http://icculus.org/al_osx/)
  ifeq ($(USE_OPENAL),1)
    ifneq ($(USE_OPENAL_DLOPEN),1)
      CLIENT_LDFLAGS += -framework OpenAL
    endif
  endif

else # ifeq darwin


#############################################################################
# SETUP AND BUILD -- MINGW32
#############################################################################

ifeq ($(PLATFORM),mingw32)

  CC=gcc
  WINDRES=windres

  ARCH=x86

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes

  ifeq ($(USE_OPENAL),1)
    BASE_CFLAGS += -DUSE_OPENAL=1 -DUSE_OPENAL_DLOPEN=1
  endif

  GL_CFLAGS =
  MINGW_CFLAGS = -DDONT_TYPEDEF_INT32

  OPTIMIZE = -O3 -march=i686 -fomit-frame-pointer -ffast-math -falign-loops=2 \
    -funroll-loops -falign-jumps=2 -falign-functions=2 -fstrength-reduce

  DEBUG_CFLAGS=$(BASE_CFLAGS) -g -O0

  RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG $(OPTIMIZE)

  SHLIBEXT=dll
  SHLIBCFLAGS=
  SHLIBLDFLAGS=-shared $(LDFLAGS)

  BINEXT=.exe

  LDFLAGS= -mwindows -lwsock32 -lgdi32 -lwinmm -lole32
  CLIENT_LDFLAGS=

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
  else #default to i386
    ARCH=i386
  endif #alpha test


  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes

  GL_CFLAGS = -I/usr/X11R6/include

  DEBUG_CFLAGS=$(BASE_CFLAGS) -g

  ifeq ($(ARCH),axp)
    CC=gcc
    BASE_CFLAGS += -DNO_VM_COMPILED
    RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3 -ffast-math -funroll-loops \
      -fomit-frame-pointer -fexpensive-optimizations
  else
  ifeq ($(ARCH),i386)
    CC=gcc
    RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O3 -mtune=pentiumpro \
      -march=pentium -fomit-frame-pointer -pipe -ffast-math \
      -falign-loops=2 -falign-jumps=2 -falign-functions=2 \
      -funroll-loops -fstrength-reduce
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

  CLIENT_LDFLAGS=-L/usr/X11R6/$(LIB) -lGL -lX11 -lXext -lXxf86dga -lXxf86vm

else # ifeq freebsd

#############################################################################
# SETUP AND BUILD -- NETBSD
#############################################################################

ifeq ($(PLATFORM),netbsd)

  ifeq ($(shell uname -m),i386)
    ARCH=i386
  endif

  CC=gcc
  LDFLAGS=-lm
  SHLIBEXT=so
  SHLIBCFLAGS=-fPIC
  SHLIBLDFLAGS=-shared $(LDFLAGS)
  THREAD_LDFLAGS=-lpthread

  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes
  DEBUG_CFLAGS=$(BASE_CFLAGS) -g

  ifneq ($(ARCH),i386)
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

  CC=cc
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

ifeq ($(PLATFORM),SunOS)

  CC=gcc
  INSTALL=ginstall
  MKDIR=gmkdir
  COPYDIR="/usr/local/share/games/tremulous"

  ifneq (,$(findstring i86pc,$(shell uname -m)))
    ARCH=i386
  else #default to sparc
    ARCH=sparc
  endif

  ifneq ($(ARCH),i386)
    ifneq ($(ARCH),sparc)
      $(error arch $(ARCH) is currently not supported)
    endif
  endif


  BASE_CFLAGS = -Wall -fno-strict-aliasing -Wimplicit -Wstrict-prototypes -pipe

  ifeq ($(USE_SDL),1)
    BASE_CFLAGS += -DUSE_SDL_SOUND=1 $(shell sdl-config --cflags)
    GL_CFLAGS =
  else
    GL_CFLAGS = -I/usr/openwin/include
  endif

  OPTIMIZE = -O3 -ffast-math -funroll-loops

  ifeq ($(ARCH),sparc)
    OPTIMIZE = -O3 -ffast-math -falign-loops=2 \
      -falign-jumps=2 -falign-functions=2 -fstrength-reduce \
      -mtune=ultrasparc -mv8plus -mno-faster-structs \
      -funroll-loops
    BASE_CFLAGS += -DNO_VM_COMPILED
  else
  ifeq ($(ARCH),i386)
    OPTIMIZE = -O3 -march=i686  -ffast-math \
      -falign-loops=2 -falign-jumps=2 -falign-functions=2 \
      -funroll-loops -fstrength-reduce
  endif
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

  ifeq ($(ARCH),i386)
    # Solarix x86 make ...
    BASE_CFLAGS += -m32
    LDFLAGS+=-m32
  endif

else # ifeq SunOS

#############################################################################
# SETUP AND BUILD -- GENERIC
#############################################################################
  CC=cc
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
    TARGETS += $(B)/tremulous.$(ARCH)$(BINEXT)
  endif
endif

ifneq ($(BUILD_GAME_SO),0)
  TARGETS += \
    $(B)/base/cgame$(ARCH).$(SHLIBEXT) \
    $(B)/base/qagame$(ARCH).$(SHLIBEXT) \
    $(B)/base/ui$(ARCH).$(SHLIBEXT)
endif

ifneq ($(BUILD_GAME_QVM),0)
  ifneq ($(CROSS_COMPILING),1)
    TARGETS += \
      $(B)/base/vm/cgame.qvm \
      $(B)/base/vm/qagame.qvm \
      $(B)/base/vm/ui.qvm \
      qvmdeps
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
  ifeq ($(CC),gcc)
    DEPEND_CFLAGS=-MMD
  endif
endif

DO_CC=$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -o $@ -c $<
DO_SMP_CC=$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) -DSMP -o $@ -c $<
DO_BOT_CC=$(CC) $(NOTSHLIBCFLAGS) $(CFLAGS) $(BOTCFLAGS) -DBOTLIB -o $@ -c $<   # $(SHLIBCFLAGS) # bk001212
DO_DEBUG_CC=$(CC) $(NOTSHLIBCFLAGS) $(DEBUG_CFLAGS) -o $@ -c $<
DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_SHLIB_DEBUG_CC=$(CC) $(DEBUG_CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<
DO_AS=$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<
DO_DED_CC=$(CC) $(NOTSHLIBCFLAGS) -DDEDICATED $(CFLAGS) -o $@ -c $<
DO_WINDRES=$(WINDRES) -i $< -o $@

#############################################################################
# MAIN TARGETS
#############################################################################

default:build_release

debug: build_debug
release: build_release

build_debug: B=$(BD)
build_debug: makedirs tools
	$(MAKE) targets B=$(BD) CFLAGS="$(CFLAGS) $(DEBUG_CFLAGS) $(DEPEND_CFLAGS)"
	$(MAKE) -C ../master debug

build_release: B=$(BR)
build_release: makedirs tools
	$(MAKE) targets B=$(BR) CFLAGS="$(CFLAGS) $(RELEASE_CFLAGS) $(DEPEND_CFLAGS)"
	$(MAKE) -C ../master release

#Build both debug and release builds
all:build_debug build_release

targets: $(TARGETS)

makedirs:
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/client ];then $(MKDIR) $(B)/client;fi
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

Q3LCC=../tools/q3lcc$(BINEXT)
Q3ASM=../tools/q3asm$(BINEXT)

ifeq ($(CROSS_COMPILING),1)
tools:
	echo QVM tools not built when cross-compiling
else
tools:
	$(MAKE) -C ../tools/lcc install
	$(MAKE) -C ../tools/asm install
endif

DO_Q3LCC=$(Q3LCC) -o $@ $<

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
  $(B)/client/msg.o \
  $(B)/client/net_chan.o \
  $(B)/client/huffman.o \
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
  \
  $(B)/client/qal.o \
  $(B)/client/snd_openal.o \
  \
  $(B)/client/sv_bot.o \
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
  $(B)/client/be_aas_bspq3.o \
  $(B)/client/be_aas_cluster.o \
  $(B)/client/be_aas_debug.o \
  $(B)/client/be_aas_entity.o \
  $(B)/client/be_aas_file.o \
  $(B)/client/be_aas_main.o \
  $(B)/client/be_aas_move.o \
  $(B)/client/be_aas_optimize.o \
  $(B)/client/be_aas_reach.o \
  $(B)/client/be_aas_route.o \
  $(B)/client/be_aas_routealt.o \
  $(B)/client/be_aas_sample.o \
  $(B)/client/be_ai_char.o \
  $(B)/client/be_ai_chat.o \
  $(B)/client/be_ai_gen.o \
  $(B)/client/be_ai_goal.o \
  $(B)/client/be_ai_move.o \
  $(B)/client/be_ai_weap.o \
  $(B)/client/be_ai_weight.o \
  $(B)/client/be_ea.o \
  $(B)/client/be_interface.o \
  $(B)/client/l_crc.o \
  $(B)/client/l_libvar.o \
  $(B)/client/l_log.o \
  $(B)/client/l_memory.o \
  $(B)/client/l_precomp.o \
  $(B)/client/l_script.o \
  $(B)/client/l_struct.o \
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

ifeq ($(ARCH),i386)
  Q3OBJ += $(B)/client/vm_x86.o
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/ftola.o \
    $(B)/client/snapvectora.o
endif
ifeq ($(ARCH),x86)
  Q3OBJ += $(B)/client/vm_x86.o
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/ftola.o \
    $(B)/client/snapvectora.o
endif
ifeq ($(ARCH),x86_64)
  Q3OBJ += $(B)/client/vm_x86_64.o

  #FIXME: why do these need to be here?
  Q3OBJ += \
    $(B)/client/snd_mixa.o \
    $(B)/client/matha.o \
    $(B)/client/ftola.o \
    $(B)/client/snapvectora.o
endif

ifeq ($(ARCH),ppc)
  ifneq ($(VM_PPC),)
    Q3OBJ += $(B)/client/$(VM_PPC).o
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
    $(B)/client/linux_glimp_smp.o \
    $(B)/client/sdl_glimp_smp.o
endif

$(B)/tremulous.$(ARCH)$(BINEXT): $(Q3OBJ) $(Q3POBJ) $(LIBSDLMAIN)
	$(CC)  -o $@ $(Q3OBJ) $(Q3POBJ) $(CLIENT_LDFLAGS) $(LDFLAGS) $(LIBSDLMAIN)

$(B)/tremulous-smp.$(ARCH)$(BINEXT): $(Q3OBJ) $(Q3POBJ_SMP) $(LIBSDLMAIN)
	$(CC)  -o $@ $(Q3OBJ) $(Q3POBJ_SMP) $(CLIENT_LDFLAGS) \
		$(THREAD_LDFLAGS) $(LDFLAGS) $(LIBSDLMAIN)

ifneq ($(strip $(LIBSDLMAIN)),)
ifneq ($(strip $(LIBSDLMAINSRC)),)
$(LIBSDLMAIN) : $(LIBSDLMAINSRC)
	cp $< $@
	ranlib $@
endif
endif

$(B)/client/cl_cgame.o : $(CDIR)/cl_cgame.c; $(DO_CC)
$(B)/client/cl_cin.o : $(CDIR)/cl_cin.c; $(DO_CC)
$(B)/client/cl_console.o : $(CDIR)/cl_console.c; $(DO_CC)
$(B)/client/cl_input.o : $(CDIR)/cl_input.c; $(DO_CC)
$(B)/client/cl_keys.o : $(CDIR)/cl_keys.c; $(DO_CC)
$(B)/client/cl_main.o : $(CDIR)/cl_main.c; $(DO_CC)
$(B)/client/cl_net_chan.o : $(CDIR)/cl_net_chan.c; $(DO_CC)
$(B)/client/cl_parse.o : $(CDIR)/cl_parse.c; $(DO_CC)
$(B)/client/cl_scrn.o : $(CDIR)/cl_scrn.c; $(DO_CC)
$(B)/client/cl_ui.o : $(CDIR)/cl_ui.c; $(DO_CC)
$(B)/client/cl_avi.o : $(CDIR)/cl_avi.c; $(DO_CC)
$(B)/client/snd_adpcm.o : $(CDIR)/snd_adpcm.c; $(DO_CC)
$(B)/client/snd_dma.o : $(CDIR)/snd_dma.c; $(DO_CC)
$(B)/client/snd_mem.o : $(CDIR)/snd_mem.c; $(DO_CC)
$(B)/client/snd_mix.o : $(CDIR)/snd_mix.c; $(DO_CC)
$(B)/client/snd_wavelet.o : $(CDIR)/snd_wavelet.c; $(DO_CC)

$(B)/client/snd_main.o : $(CDIR)/snd_main.c; $(DO_CC)
$(B)/client/snd_codec.o : $(CDIR)/snd_codec.c; $(DO_CC)
$(B)/client/snd_codec_wav.o : $(CDIR)/snd_codec_wav.c; $(DO_CC)

$(B)/client/qal.o : $(CDIR)/qal.c; $(DO_CC)
$(B)/client/snd_openal.o : $(CDIR)/snd_openal.c; $(DO_CC)

$(B)/client/sv_bot.o : $(SDIR)/sv_bot.c; $(DO_CC)
$(B)/client/sv_client.o : $(SDIR)/sv_client.c; $(DO_CC)
$(B)/client/sv_ccmds.o : $(SDIR)/sv_ccmds.c; $(DO_CC)
$(B)/client/sv_game.o : $(SDIR)/sv_game.c; $(DO_CC)
$(B)/client/sv_init.o : $(SDIR)/sv_init.c; $(DO_CC)
$(B)/client/sv_main.o : $(SDIR)/sv_main.c; $(DO_CC)
$(B)/client/sv_net_chan.o : $(SDIR)/sv_net_chan.c; $(DO_CC)
$(B)/client/sv_snapshot.o : $(SDIR)/sv_snapshot.c; $(DO_CC)
$(B)/client/sv_world.o : $(SDIR)/sv_world.c; $(DO_CC)
$(B)/client/cm_trace.o : $(CMDIR)/cm_trace.c; $(DO_CC)
$(B)/client/cm_load.o : $(CMDIR)/cm_load.c; $(DO_CC)
$(B)/client/cm_test.o : $(CMDIR)/cm_test.c; $(DO_CC)
$(B)/client/cm_patch.o : $(CMDIR)/cm_patch.c; $(DO_CC)
$(B)/client/cm_polylib.o : $(CMDIR)/cm_polylib.c; $(DO_CC)
$(B)/client/cmd.o : $(CMDIR)/cmd.c; $(DO_CC)
$(B)/client/common.o : $(CMDIR)/common.c; $(DO_CC)
$(B)/client/cvar.o : $(CMDIR)/cvar.c; $(DO_CC)
$(B)/client/files.o : $(CMDIR)/files.c; $(DO_CC)
$(B)/client/md4.o : $(CMDIR)/md4.c; $(DO_CC)
$(B)/client/msg.o : $(CMDIR)/msg.c; $(DO_CC)
$(B)/client/net_chan.o : $(CMDIR)/net_chan.c; $(DO_CC)
$(B)/client/huffman.o : $(CMDIR)/huffman.c; $(DO_CC)
$(B)/client/q_shared.o : $(CMDIR)/q_shared.c; $(DO_CC)
$(B)/client/q_math.o : $(CMDIR)/q_math.c; $(DO_CC)

$(B)/client/be_aas_bspq3.o : $(BLIBDIR)/be_aas_bspq3.c; $(DO_BOT_CC)
$(B)/client/be_aas_cluster.o : $(BLIBDIR)/be_aas_cluster.c; $(DO_BOT_CC)
$(B)/client/be_aas_debug.o : $(BLIBDIR)/be_aas_debug.c; $(DO_BOT_CC)
$(B)/client/be_aas_entity.o : $(BLIBDIR)/be_aas_entity.c; $(DO_BOT_CC)
$(B)/client/be_aas_file.o : $(BLIBDIR)/be_aas_file.c; $(DO_BOT_CC)
$(B)/client/be_aas_main.o : $(BLIBDIR)/be_aas_main.c; $(DO_BOT_CC)
$(B)/client/be_aas_move.o : $(BLIBDIR)/be_aas_move.c; $(DO_BOT_CC)
$(B)/client/be_aas_optimize.o : $(BLIBDIR)/be_aas_optimize.c; $(DO_BOT_CC)
$(B)/client/be_aas_reach.o : $(BLIBDIR)/be_aas_reach.c; $(DO_BOT_CC)
$(B)/client/be_aas_route.o : $(BLIBDIR)/be_aas_route.c; $(DO_BOT_CC)
$(B)/client/be_aas_routealt.o : $(BLIBDIR)/be_aas_routealt.c; $(DO_BOT_CC)
$(B)/client/be_aas_sample.o : $(BLIBDIR)/be_aas_sample.c; $(DO_BOT_CC)
$(B)/client/be_ai_char.o : $(BLIBDIR)/be_ai_char.c; $(DO_BOT_CC)
$(B)/client/be_ai_chat.o : $(BLIBDIR)/be_ai_chat.c; $(DO_BOT_CC)
$(B)/client/be_ai_gen.o : $(BLIBDIR)/be_ai_gen.c; $(DO_BOT_CC)
$(B)/client/be_ai_goal.o : $(BLIBDIR)/be_ai_goal.c; $(DO_BOT_CC)
$(B)/client/be_ai_move.o : $(BLIBDIR)/be_ai_move.c; $(DO_BOT_CC)
$(B)/client/be_ai_weap.o : $(BLIBDIR)/be_ai_weap.c; $(DO_BOT_CC)
$(B)/client/be_ai_weight.o : $(BLIBDIR)/be_ai_weight.c; $(DO_BOT_CC)
$(B)/client/be_ea.o : $(BLIBDIR)/be_ea.c; $(DO_BOT_CC)
$(B)/client/be_interface.o : $(BLIBDIR)/be_interface.c; $(DO_BOT_CC)
$(B)/client/l_crc.o : $(BLIBDIR)/l_crc.c; $(DO_BOT_CC)
$(B)/client/l_libvar.o : $(BLIBDIR)/l_libvar.c; $(DO_BOT_CC)
$(B)/client/l_log.o : $(BLIBDIR)/l_log.c; $(DO_BOT_CC)
$(B)/client/l_memory.o : $(BLIBDIR)/l_memory.c; $(DO_BOT_CC)
$(B)/client/l_precomp.o : $(BLIBDIR)/l_precomp.c; $(DO_BOT_CC)
$(B)/client/l_script.o : $(BLIBDIR)/l_script.c; $(DO_BOT_CC)
$(B)/client/l_struct.o : $(BLIBDIR)/l_struct.c; $(DO_BOT_CC)

$(B)/client/jcapimin.o : $(JPDIR)/jcapimin.c; $(DO_CC)
$(B)/client/jchuff.o : $(JPDIR)/jchuff.c; $(DO_CC)
$(B)/client/jcinit.o : $(JPDIR)/jcinit.c; $(DO_CC)
$(B)/client/jccoefct.o : $(JPDIR)/jccoefct.c; $(DO_CC)
$(B)/client/jccolor.o : $(JPDIR)/jccolor.c; $(DO_CC)
$(B)/client/jfdctflt.o : $(JPDIR)/jfdctflt.c; $(DO_CC)
$(B)/client/jcdctmgr.o : $(JPDIR)/jcdctmgr.c; $(DO_CC)
$(B)/client/jcmainct.o : $(JPDIR)/jcmainct.c; $(DO_CC)
$(B)/client/jcmarker.o : $(JPDIR)/jcmarker.c; $(DO_CC)
$(B)/client/jcmaster.o : $(JPDIR)/jcmaster.c; $(DO_CC)
$(B)/client/jcomapi.o : $(JPDIR)/jcomapi.c; $(DO_CC)
$(B)/client/jcparam.o : $(JPDIR)/jcparam.c;  $(DO_CC)
$(B)/client/jcprepct.o : $(JPDIR)/jcprepct.c; $(DO_CC)
$(B)/client/jcsample.o : $(JPDIR)/jcsample.c; $(DO_CC)

$(B)/client/jdapimin.o : $(JPDIR)/jdapimin.c; $(DO_CC)
$(B)/client/jdapistd.o : $(JPDIR)/jdapistd.c; $(DO_CC)
$(B)/client/jdatasrc.o : $(JPDIR)/jdatasrc.c; $(DO_CC)
$(B)/client/jdcoefct.o : $(JPDIR)/jdcoefct.c; $(DO_CC)
$(B)/client/jdcolor.o : $(JPDIR)/jdcolor.c; $(DO_CC)
$(B)/client/jcphuff.o : $(JPDIR)/jcphuff.c; $(DO_CC)
$(B)/client/jddctmgr.o : $(JPDIR)/jddctmgr.c; $(DO_CC)
$(B)/client/jdhuff.o : $(JPDIR)/jdhuff.c; $(DO_CC)
$(B)/client/jdinput.o : $(JPDIR)/jdinput.c; $(DO_CC)
$(B)/client/jdmainct.o : $(JPDIR)/jdmainct.c; $(DO_CC)
$(B)/client/jdmarker.o : $(JPDIR)/jdmarker.c; $(DO_CC)
$(B)/client/jdmaster.o : $(JPDIR)/jdmaster.c; $(DO_CC)
$(B)/client/jdpostct.o : $(JPDIR)/jdpostct.c; $(DO_CC)
$(B)/client/jdsample.o : $(JPDIR)/jdsample.c; $(DO_CC)
$(B)/client/jdtrans.o : $(JPDIR)/jdtrans.c; $(DO_CC)
$(B)/client/jerror.o : $(JPDIR)/jerror.c; $(DO_CC) $(GL_CFLAGS) $(MINGW_CFLAGS)
$(B)/client/jidctflt.o : $(JPDIR)/jidctflt.c; $(DO_CC)
$(B)/client/jmemmgr.o : $(JPDIR)/jmemmgr.c; $(DO_CC)
$(B)/client/jmemnobs.o : $(JPDIR)/jmemnobs.c; $(DO_CC)  $(GL_CFLAGS) $(MINGW_CFLAGS)
$(B)/client/jutils.o : $(JPDIR)/jutils.c; $(DO_CC)

$(B)/client/tr_bsp.o : $(RDIR)/tr_bsp.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_animation.o : $(RDIR)/tr_animation.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_backend.o : $(RDIR)/tr_backend.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_cmds.o : $(RDIR)/tr_cmds.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_curve.o : $(RDIR)/tr_curve.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_flares.o : $(RDIR)/tr_flares.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_font.o : $(RDIR)/tr_font.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_image.o : $(RDIR)/tr_image.c; $(DO_CC)   $(GL_CFLAGS) $(MINGW_CFLAGS)
$(B)/client/tr_init.o : $(RDIR)/tr_init.c; $(DO_CC)    $(GL_CFLAGS)
$(B)/client/tr_light.o : $(RDIR)/tr_light.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_main.o : $(RDIR)/tr_main.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_marks.o : $(RDIR)/tr_marks.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_mesh.o : $(RDIR)/tr_mesh.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_model.o : $(RDIR)/tr_model.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_noise.o : $(RDIR)/tr_noise.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_scene.o : $(RDIR)/tr_scene.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_shade.o : $(RDIR)/tr_shade.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_shader.o : $(RDIR)/tr_shader.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_shade_calc.o : $(RDIR)/tr_shade_calc.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_shadows.o : $(RDIR)/tr_shadows.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_sky.o : $(RDIR)/tr_sky.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_smp.o : $(RDIR)/tr_smp.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_stripify.o : $(RDIR)/tr_stripify.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_subdivide.o : $(RDIR)/tr_subdivide.c; $(DO_CC)   $(GL_CFLAGS)
$(B)/client/tr_surface.o : $(RDIR)/tr_surface.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/tr_world.o : $(RDIR)/tr_world.c; $(DO_CC)   $(GL_CFLAGS)

$(B)/client/unix_qgl.o : $(UDIR)/unix_qgl.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/unix_main.o : $(UDIR)/unix_main.c; $(DO_CC)
$(B)/client/unix_net.o : $(UDIR)/unix_net.c; $(DO_CC)
$(B)/client/unix_shared.o : $(UDIR)/unix_shared.c; $(DO_CC)
$(B)/client/irix_glimp.o : $(UDIR)/irix_glimp.c; $(DO_CC)
$(B)/client/irix_glimp_smp.o : $(UDIR)/irix_glimp.c; $(DO_SMP_CC)
$(B)/client/irix_snd.o : $(UDIR)/irix_snd.c; $(DO_CC)
$(B)/client/irix_input.o : $(UDIR)/irix_input.c; $(DO_CC)
$(B)/client/linux_signals.o : $(UDIR)/linux_signals.c; $(DO_CC) $(GL_CFLAGS)
$(B)/client/linux_glimp.o : $(UDIR)/linux_glimp.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/sdl_glimp.o : $(UDIR)/sdl_glimp.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/linux_glimp_smp.o : $(UDIR)/linux_glimp.c; $(DO_SMP_CC)  $(GL_CFLAGS)
$(B)/client/sdl_glimp_smp.o : $(UDIR)/sdl_glimp.c; $(DO_SMP_CC)  $(GL_CFLAGS)
$(B)/client/linux_joystick.o : $(UDIR)/linux_joystick.c; $(DO_CC)
$(B)/client/linux_qgl.o : $(UDIR)/linux_qgl.c; $(DO_CC)  $(GL_CFLAGS)
$(B)/client/linux_input.o : $(UDIR)/linux_input.c; $(DO_CC)
$(B)/client/linux_snd.o : $(UDIR)/linux_snd.c; $(DO_CC)
$(B)/client/sdl_snd.o : $(UDIR)/sdl_snd.c; $(DO_CC)
$(B)/client/snd_mixa.o : $(UDIR)/snd_mixa.s; $(DO_AS)
$(B)/client/matha.o : $(UDIR)/matha.s; $(DO_AS)
$(B)/client/ftola.o : $(UDIR)/ftola.s; $(DO_AS)
$(B)/client/snapvectora.o : $(UDIR)/snapvectora.s; $(DO_AS)

$(B)/client/win_gamma.o : $(W32DIR)/win_gamma.c; $(DO_CC)
$(B)/client/win_glimp.o : $(W32DIR)/win_glimp.c; $(DO_CC)
$(B)/client/win_input.o : $(W32DIR)/win_input.c; $(DO_CC)
$(B)/client/win_main.o : $(W32DIR)/win_main.c; $(DO_CC)
$(B)/client/win_net.o : $(W32DIR)/win_net.c; $(DO_CC)
$(B)/client/win_qgl.o : $(W32DIR)/win_qgl.c; $(DO_CC)
$(B)/client/win_shared.o : $(W32DIR)/win_shared.c; $(DO_CC)
$(B)/client/win_snd.o : $(W32DIR)/win_snd.c; $(DO_CC)
$(B)/client/win_syscon.o : $(W32DIR)/win_syscon.c; $(DO_CC)
$(B)/client/win_wndproc.o : $(W32DIR)/win_wndproc.c; $(DO_CC)
$(B)/client/win_resource.o : $(W32DIR)/winquake.rc; $(DO_WINDRES)

$(B)/client/vm_x86.o : $(CMDIR)/vm_x86.c; $(DO_CC)
$(B)/client/vm_x86_64.o : $(CMDIR)/vm_x86_64.c; $(DO_CC)
ifneq ($(VM_PPC),)
$(B)/client/$(VM_PPC).o : $(CMDIR)/$(VM_PPC).c; $(DO_CC)
endif

$(B)/client/unzip.o : $(CMDIR)/unzip.c; $(DO_CC)
$(B)/client/vm.o : $(CMDIR)/vm.c; $(DO_CC)
$(B)/client/vm_interpreted.o : $(CMDIR)/vm_interpreted.c; $(DO_CC)

#############################################################################
# DEDICATED SERVER
#############################################################################

Q3DOBJ = \
  $(B)/ded/sv_bot.o \
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
  \
  $(B)/ded/q_math.o \
  $(B)/ded/q_shared.o \
  \
  $(B)/ded/unzip.o \
  $(B)/ded/vm.o \
  $(B)/ded/vm_interpreted.o \
  \
  $(B)/ded/be_aas_bspq3.o \
  $(B)/ded/be_aas_cluster.o \
  $(B)/ded/be_aas_debug.o \
  $(B)/ded/be_aas_entity.o \
  $(B)/ded/be_aas_file.o \
  $(B)/ded/be_aas_main.o \
  $(B)/ded/be_aas_move.o \
  $(B)/ded/be_aas_optimize.o \
  $(B)/ded/be_aas_reach.o \
  $(B)/ded/be_aas_route.o \
  $(B)/ded/be_aas_routealt.o \
  $(B)/ded/be_aas_sample.o \
  $(B)/ded/be_ai_char.o \
  $(B)/ded/be_ai_chat.o \
  $(B)/ded/be_ai_gen.o \
  $(B)/ded/be_ai_goal.o \
  $(B)/ded/be_ai_move.o \
  $(B)/ded/be_ai_weap.o \
  $(B)/ded/be_ai_weight.o \
  $(B)/ded/be_ea.o \
  $(B)/ded/be_interface.o \
  $(B)/ded/l_crc.o \
  $(B)/ded/l_libvar.o \
  $(B)/ded/l_log.o \
  $(B)/ded/l_memory.o \
  $(B)/ded/l_precomp.o \
  $(B)/ded/l_script.o \
  $(B)/ded/l_struct.o \
  \
  $(B)/ded/linux_signals.o \
  $(B)/ded/unix_main.o \
  $(B)/ded/unix_net.o \
  $(B)/ded/unix_shared.o \
  \
  $(B)/ded/null_client.o \
  $(B)/ded/null_input.o \
  $(B)/ded/null_snddma.o

ifeq ($(ARCH),i386)
  Q3DOBJ += $(B)/ded/vm_x86.o $(B)/ded/ftola.o \
      $(B)/ded/snapvectora.o $(B)/ded/matha.o
endif

ifeq ($(ARCH),x86_64)
  Q3DOBJ += $(B)/ded/vm_x86_64.o
endif

ifeq ($(ARCH),ppc)
  ifneq ($(VM_PPC),)
    Q3DOBJ += $(B)/ded/$(VM_PPC).o
  endif
endif

$(B)/tremded.$(ARCH)$(BINEXT): $(Q3DOBJ)
	$(CC) -o $@ $(Q3DOBJ) $(LDFLAGS)

$(B)/ded/sv_bot.o : $(SDIR)/sv_bot.c; $(DO_DED_CC)
$(B)/ded/sv_client.o : $(SDIR)/sv_client.c; $(DO_DED_CC)
$(B)/ded/sv_ccmds.o : $(SDIR)/sv_ccmds.c; $(DO_DED_CC)
$(B)/ded/sv_game.o : $(SDIR)/sv_game.c; $(DO_DED_CC)
$(B)/ded/sv_init.o : $(SDIR)/sv_init.c; $(DO_DED_CC)
$(B)/ded/sv_main.o : $(SDIR)/sv_main.c; $(DO_DED_CC)
$(B)/ded/sv_net_chan.o : $(SDIR)/sv_net_chan.c; $(DO_DED_CC)
$(B)/ded/sv_snapshot.o : $(SDIR)/sv_snapshot.c; $(DO_DED_CC)
$(B)/ded/sv_world.o : $(SDIR)/sv_world.c; $(DO_DED_CC)
$(B)/ded/cm_load.o : $(CMDIR)/cm_load.c; $(DO_DED_CC)
$(B)/ded/cm_polylib.o : $(CMDIR)/cm_polylib.c; $(DO_DED_CC)
$(B)/ded/cm_test.o : $(CMDIR)/cm_test.c; $(DO_DED_CC)
$(B)/ded/cm_trace.o : $(CMDIR)/cm_trace.c; $(DO_DED_CC)
$(B)/ded/cm_patch.o : $(CMDIR)/cm_patch.c; $(DO_DED_CC)
$(B)/ded/cmd.o : $(CMDIR)/cmd.c; $(DO_DED_CC)
$(B)/ded/common.o : $(CMDIR)/common.c; $(DO_DED_CC)
$(B)/ded/cvar.o : $(CMDIR)/cvar.c; $(DO_DED_CC)
$(B)/ded/files.o : $(CMDIR)/files.c; $(DO_DED_CC)
$(B)/ded/md4.o : $(CMDIR)/md4.c; $(DO_DED_CC)
$(B)/ded/msg.o : $(CMDIR)/msg.c; $(DO_DED_CC)
$(B)/ded/net_chan.o : $(CMDIR)/net_chan.c; $(DO_DED_CC)
$(B)/ded/huffman.o : $(CMDIR)/huffman.c; $(DO_DED_CC)
$(B)/ded/q_shared.o : $(CMDIR)/q_shared.c; $(DO_DED_CC)
$(B)/ded/q_math.o : $(CMDIR)/q_math.c; $(DO_DED_CC)

$(B)/ded/be_aas_bspq3.o : $(BLIBDIR)/be_aas_bspq3.c; $(DO_BOT_CC)
$(B)/ded/be_aas_cluster.o : $(BLIBDIR)/be_aas_cluster.c; $(DO_BOT_CC)
$(B)/ded/be_aas_debug.o : $(BLIBDIR)/be_aas_debug.c; $(DO_BOT_CC)
$(B)/ded/be_aas_entity.o : $(BLIBDIR)/be_aas_entity.c; $(DO_BOT_CC)
$(B)/ded/be_aas_file.o : $(BLIBDIR)/be_aas_file.c; $(DO_BOT_CC)
$(B)/ded/be_aas_main.o : $(BLIBDIR)/be_aas_main.c; $(DO_BOT_CC)
$(B)/ded/be_aas_move.o : $(BLIBDIR)/be_aas_move.c; $(DO_BOT_CC)
$(B)/ded/be_aas_optimize.o : $(BLIBDIR)/be_aas_optimize.c; $(DO_BOT_CC)
$(B)/ded/be_aas_reach.o : $(BLIBDIR)/be_aas_reach.c; $(DO_BOT_CC)
$(B)/ded/be_aas_route.o : $(BLIBDIR)/be_aas_route.c; $(DO_BOT_CC)
$(B)/ded/be_aas_routealt.o : $(BLIBDIR)/be_aas_routealt.c; $(DO_BOT_CC)
$(B)/ded/be_aas_sample.o : $(BLIBDIR)/be_aas_sample.c; $(DO_BOT_CC)
$(B)/ded/be_ai_char.o : $(BLIBDIR)/be_ai_char.c; $(DO_BOT_CC)
$(B)/ded/be_ai_chat.o : $(BLIBDIR)/be_ai_chat.c; $(DO_BOT_CC)
$(B)/ded/be_ai_gen.o : $(BLIBDIR)/be_ai_gen.c; $(DO_BOT_CC)
$(B)/ded/be_ai_goal.o : $(BLIBDIR)/be_ai_goal.c; $(DO_BOT_CC)
$(B)/ded/be_ai_move.o : $(BLIBDIR)/be_ai_move.c; $(DO_BOT_CC)
$(B)/ded/be_ai_weap.o : $(BLIBDIR)/be_ai_weap.c; $(DO_BOT_CC)
$(B)/ded/be_ai_weight.o : $(BLIBDIR)/be_ai_weight.c; $(DO_BOT_CC)
$(B)/ded/be_ea.o : $(BLIBDIR)/be_ea.c; $(DO_BOT_CC)
$(B)/ded/be_interface.o : $(BLIBDIR)/be_interface.c; $(DO_BOT_CC)
$(B)/ded/l_crc.o : $(BLIBDIR)/l_crc.c; $(DO_BOT_CC)
$(B)/ded/l_libvar.o : $(BLIBDIR)/l_libvar.c; $(DO_BOT_CC)
$(B)/ded/l_log.o : $(BLIBDIR)/l_log.c; $(DO_BOT_CC)
$(B)/ded/l_memory.o : $(BLIBDIR)/l_memory.c; $(DO_BOT_CC)
$(B)/ded/l_precomp.o : $(BLIBDIR)/l_precomp.c; $(DO_BOT_CC)
$(B)/ded/l_script.o : $(BLIBDIR)/l_script.c; $(DO_BOT_CC)
$(B)/ded/l_struct.o : $(BLIBDIR)/l_struct.c; $(DO_BOT_CC)

$(B)/ded/linux_signals.o : $(UDIR)/linux_signals.c; $(DO_DED_CC)
$(B)/ded/unix_main.o : $(UDIR)/unix_main.c; $(DO_DED_CC)
$(B)/ded/unix_net.o : $(UDIR)/unix_net.c; $(DO_DED_CC)
$(B)/ded/unix_shared.o : $(UDIR)/unix_shared.c; $(DO_DED_CC)

$(B)/ded/null_client.o : $(NDIR)/null_client.c; $(DO_DED_CC)
$(B)/ded/null_input.o : $(NDIR)/null_input.c; $(DO_DED_CC)
$(B)/ded/null_snddma.o : $(NDIR)/null_snddma.c; $(DO_DED_CC)
$(B)/ded/unzip.o : $(CMDIR)/unzip.c; $(DO_DED_CC)
$(B)/ded/vm.o : $(CMDIR)/vm.c; $(DO_DED_CC)
$(B)/ded/vm_interpreted.o : $(CMDIR)/vm_interpreted.c; $(DO_DED_CC)

$(B)/ded/ftola.o : $(UDIR)/ftola.s; $(DO_AS)
$(B)/ded/snapvectora.o : $(UDIR)/snapvectora.s; $(DO_AS)
$(B)/ded/matha.o : $(UDIR)/matha.s; $(DO_AS)

$(B)/ded/vm_x86.o : $(CMDIR)/vm_x86.c; $(DO_DED_CC)
$(B)/ded/vm_x86_64.o : $(CMDIR)/vm_x86_64.c; $(DO_DED_CC)
ifneq ($(VM_PPC),)
$(B)/ded/$(VM_PPC).o : $(CMDIR)/$(VM_PPC).c; $(DO_DED_CC)
endif



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
  $(B)/base/ui/ui_shared.o \
  \
  $(B)/base/qcommon/q_math.o \
  $(B)/base/qcommon/q_shared.o

CGOBJ = $(CGOBJ_) $(B)/base/cgame/cg_syscalls.o
CGVMOBJ = $(CGOBJ_:%.o=%.asm) $(B)/base/game/bg_lib.asm

$(B)/base/cgame$(ARCH).$(SHLIBEXT) : $(CGOBJ)
	$(CC) $(SHLIBLDFLAGS) -o $@ $(CGOBJ)

$(B)/base/vm/cgame.qvm: $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm
	$(Q3ASM) -o $@ $(CGVMOBJ) $(CGDIR)/cg_syscalls.asm



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
  \
  $(B)/base/qcommon/q_math.o \
  $(B)/base/qcommon/q_shared.o

GOBJ = $(GOBJ_) $(B)/base/game/g_syscalls.o
GVMOBJ = $(GOBJ_:%.o=%.asm) $(B)/base/game/bg_lib.asm

$(B)/base/qagame$(ARCH).$(SHLIBEXT) : $(GOBJ)
	$(CC) $(SHLIBLDFLAGS) -o $@ $(GOBJ)

$(B)/base/vm/qagame.qvm: $(GVMOBJ) $(GDIR)/g_syscalls.asm
	$(Q3ASM) -o $@ $(GVMOBJ) $(GDIR)/g_syscalls.asm



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
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(UIOBJ)

$(B)/base/vm/ui.qvm: $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm
	$(Q3ASM) -o $@ $(UIVMOBJ) $(UIDIR)/ui_syscalls.asm


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

copyfiles: build_release
	@if [ ! -d $(COPYDIR)/baseq3 ]; then echo "You need to set COPYDIR to where you installed Trem!"; false; fi
	$(INSTALL) -s -m 0755 $(BR)/tremulous.$(ARCH)$(BINEXT) $(COPYDIR)/tremulous.$(ARCH)$(BINEXT)

	@if [ -f $(BR)/tremded.$(ARCH)$(BINEXT) ]; then \
		$(INSTALL) -s -m 0755 $(BR)/tremded.$(ARCH)$(BINEXT) $(COPYDIR)/tremded.$(ARCH)$(BINEXT); \
	fi
	-$(MKDIR) -p -m 0755 $(COPYDIR)/base
	$(INSTALL) -s -m 0755 $(BR)/base/cgame$(ARCH).$(SHLIBEXT) \
					$(COPYDIR)/base/.
	$(INSTALL) -s -m 0755 $(BR)/base/qagame$(ARCH).$(SHLIBEXT) \
					$(COPYDIR)/base/.
	$(INSTALL) -s -m 0755 $(BR)/base/ui$(ARCH).$(SHLIBEXT) \
					$(COPYDIR)/base/.

clean:clean-debug clean-release
	$(MAKE) -C setup clean
	$(MAKE) -C ../master clean

clean2:
	if [ -d $(B) ];then (find $(B) -name '*.d' -exec rm {} \;)fi
	rm -f $(Q3OBJ) $(Q3POBJ) $(Q3POBJ_SMP) $(Q3DOBJ) \
		$(GOBJ) $(CGOBJ) $(UIOBJ) \
		$(GVMOBJ) $(CGVMOBJ) $(UIVMOBJ)
	rm -f $(TARGETS)

clean-debug:
	$(MAKE) clean2 B=$(BD) CFLAGS="$(DEBUG_CFLAGS)"

clean-release:
	$(MAKE) clean2 B=$(BR) CFLAGS="$(RELEASE_CFLAGS)"

distclean: clean
	$(MAKE) -C ../tools/asm clean uninstall
	$(MAKE) -C ../tools/lcc clean uninstall

installer: build_release
	$(MAKE) VERSION=$(VERSION) -C setup

#############################################################################
# DEPENDENCIES
#############################################################################

D_FILES=$(shell find . -name '*.d')

$(B)/base/vm/vm.d: $(GOBJ) $(CGOBJ) $(UIOBJ)
	-rm -f $@
	find $(B)/base -iname '*.d' | xargs sed -e 's/\.o/\.asm/g' > $@

qvmdeps: $(B)/base/vm/vm.d

ifneq ($(strip $(D_FILES)),)
  include $(D_FILES)
endif
