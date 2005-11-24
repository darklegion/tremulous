#
# Quake3 Unix Makefile
#
# GNU Make required
#


# --Makefile variables--
MOUNT_DIR=./src
Q3A_DIR=/home/tma/.q3a
MOD_DIR=tremulous-dev

# --object list--
GOBJ = \
	$(GDIRNAME)/g_main.o \
	$(GDIRNAME)/bg_misc.o \
	$(GDIRNAME)/bg_pmove.o \
	$(GDIRNAME)/bg_slidemove.o \
	$(GDIRNAME)/g_mem.o \
	$(GDIRNAME)/q_math.o \
	$(GDIRNAME)/q_shared.o \
	$(GDIRNAME)/g_active.o \
	$(GDIRNAME)/g_client.o \
	$(GDIRNAME)/g_cmds.o \
	$(GDIRNAME)/g_combat.o \
	$(GDIRNAME)/g_physics.o \
	$(GDIRNAME)/g_buildable.o \
	$(GDIRNAME)/g_misc.o \
	$(GDIRNAME)/g_missile.o \
	$(GDIRNAME)/g_mover.o \
	$(GDIRNAME)/g_session.o \
	$(GDIRNAME)/g_spawn.o \
	$(GDIRNAME)/g_svcmds.o \
	$(GDIRNAME)/g_target.o \
	$(GDIRNAME)/g_team.o \
	$(GDIRNAME)/g_trigger.o \
	$(GDIRNAME)/g_utils.o \
	$(GDIRNAME)/g_maprotation.o \
	$(GDIRNAME)/g_ptr.o \
	$(GDIRNAME)/g_weapon.o

CGOBJ = \
	$(CGDIRNAME)/cg_main.o \
	$(GDIRNAME)/bg_misc.o \
	$(GDIRNAME)/bg_pmove.o \
	$(GDIRNAME)/bg_slidemove.o \
	$(GDIRNAME)/q_math.o \
	$(GDIRNAME)/q_shared.o \
	$(CGDIRNAME)/cg_consolecmds.o \
	$(CGDIRNAME)/cg_buildable.o \
	$(CGDIRNAME)/cg_animation.o \
	$(CGDIRNAME)/cg_animmapobj.o \
	$(CGDIRNAME)/cg_draw.o \
	$(CGDIRNAME)/cg_drawtools.o \
	$(CGDIRNAME)/cg_effects.o \
	$(CGDIRNAME)/cg_ents.o \
	$(CGDIRNAME)/cg_event.o \
	$(CGDIRNAME)/cg_localents.o \
	$(CGDIRNAME)/cg_marks.o \
	$(CGDIRNAME)/cg_players.o \
	$(CGDIRNAME)/cg_playerstate.o \
	$(CGDIRNAME)/cg_predict.o \
	$(CGDIRNAME)/cg_servercmds.o \
	$(CGDIRNAME)/cg_snapshot.o \
	$(CGDIRNAME)/cg_view.o \
	$(CGDIRNAME)/cg_weapons.o \
	$(CGDIRNAME)/cg_mem.o \
	$(CGDIRNAME)/cg_scanner.o \
	$(CGDIRNAME)/cg_attachment.o \
	$(CGDIRNAME)/cg_trails.o \
	$(CGDIRNAME)/cg_particles.o \
	$(CGDIRNAME)/cg_ptr.o \
	$(UIDIRNAME)/ui_shared.o
  
UIOBJ = \
	$(UIDIRNAME)/ui_main.o \
	$(GDIRNAME)/bg_misc.o \
	$(GDIRNAME)/q_math.o \
	$(GDIRNAME)/q_shared.o \
	$(UIDIRNAME)/ui_atoms.o \
	$(UIDIRNAME)/ui_players.o \
	$(UIDIRNAME)/ui_shared.o \
	$(UIDIRNAME)/ui_gameinfo.o
  




  
# --You shouldn't have to touch anything below here--

# --general variables--
PLATFORM=$(shell uname|tr A-Z a-z)
PLATFORM_RELEASE=$(shell uname -r)

BD=debug$(ARCH)$(GLIBC)
BR=release$(ARCH)$(GLIBC)
BQ=qvm

GDIRNAME=game
CGDIRNAME=cgame
UIDIRNAME=ui
GDIR=$(MOUNT_DIR)/$(GDIRNAME)
CGDIR=$(MOUNT_DIR)/$(CGDIRNAME)
UIDIR=$(MOUNT_DIR)/$(UIDIRNAME)


# --gcc config--
ifneq (,$(findstring libc6,$(shell if [ -e /lib/libc.so.6* ];then echo libc6;fi)))
GLIBC=-glibc
else
GLIBC=
endif #libc6 test


ifneq (,$(findstring alpha,$(shell uname -m)))
ARCH=axp
RPMARCH=alpha
VENDOR=dec
else #default to i386
ARCH=i386
RPMARCH=i386
VENDOR=unknown
endif #alpha test

BASE_CFLAGS=-pipe -Wall -Werror
DEBUG_CFLAGS=$(BASE_CFLAGS) -g -pg
DEPEND_CFLAGS= -MM

ifeq ($(ARCH),axp)
CC=pgcc
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O6 -ffast-math -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
else
NEWPGCC=/usr/local/gcc-2.95.2/bin/gcc
CC=$(shell if [ -f $(NEWPGCC) ]; then echo $(NEWPGCC); else echo gcc; fi )
RELEASE_CFLAGS=$(BASE_CFLAGS) -DNDEBUG -O6 -mcpu=pentiumpro -march=pentium -fomit-frame-pointer -pipe -ffast-math -falign-loops=2 -falign-jumps=2 -falign-functions=2 -fno-strict-aliasing -fstrength-reduce 
endif

LIBEXT=a

SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-fPIC -shared

ARFLAGS=ar rv
RANLIB=ranlib

THREAD_LDFLAGS=-lpthread
LDFLAGS=-ldl -lm

SED=sed



# --qvm building config--
LCC=q3lcc
LCC_FLAGS=-DQ3_VM -S -Wf-target=bytecode
LCC_INCLUDES=-I$(CGDIR) -I$(GDIR) -I$(UIDIR)

Q3ASM=q3asm
Q3ASM_FLAGS=


# --source level flags--

SLF_GAME_FLAGS=-D__GAME__
SLF_CGAME_FLAGS=-D__CGAME__
SLF_UI_FLAGS=-D__UI__


# --main targets--
all: release debug qvm

release: ctags makedirs $(BR)/cgame$(ARCH).$(SHLIBEXT) $(BR)/qagame$(ARCH).$(SHLIBEXT) $(BR)/ui$(ARCH).$(SHLIBEXT)

debug: ctags makedirs $(BD)/cgame$(ARCH).$(SHLIBEXT) $(BD)/qagame$(ARCH).$(SHLIBEXT) $(BD)/ui$(ARCH).$(SHLIBEXT)

qvm: ctags makedirs $(BQ)/cgame.qvm $(BQ)/qagame.qvm $(BQ)/ui.qvm

makedirs:
	@if [ ! -d $(BR) ];then mkdir $(BR);fi
	@if [ ! -d $(BR)/$(GDIRNAME) ];then mkdir $(BR)/$(GDIRNAME);fi
	@if [ ! -d $(BR)/$(CGDIRNAME) ];then mkdir $(BR)/$(CGDIRNAME);fi
	@if [ ! -d $(BR)/$(UIDIRNAME) ];then mkdir $(BR)/$(UIDIRNAME);fi
	@if [ ! -d $(BD) ];then mkdir $(BD);fi
	@if [ ! -d $(BD)/$(GDIRNAME) ];then mkdir $(BD)/$(GDIRNAME);fi
	@if [ ! -d $(BD)/$(CGDIRNAME) ];then mkdir $(BD)/$(CGDIRNAME);fi
	@if [ ! -d $(BD)/$(UIDIRNAME) ];then mkdir $(BD)/$(UIDIRNAME);fi
	@if [ ! -d $(BQ) ];then mkdir $(BQ);fi
	@if [ ! -d $(BQ)/$(GDIRNAME) ];then mkdir $(BQ)/$(GDIRNAME);fi
	@if [ ! -d $(BQ)/$(CGDIRNAME) ];then mkdir $(BQ)/$(CGDIRNAME);fi
	@if [ ! -d $(BQ)/$(UIDIRNAME) ];then mkdir $(BQ)/$(UIDIRNAME);fi

ctags:
	ctags -f tags -R src/* ui/menudef.h

# --object lists for each build type--
GQVMOBJ = $(GOBJ:%.o=$(BQ)/%.asm)
GROBJ = $(GOBJ:%.o=$(BR)/%.o) $(BR)/$(GDIRNAME)/g_syscalls.o
GDOBJ = $(GOBJ:%.o=$(BD)/%.o) $(BD)/$(GDIRNAME)/g_syscalls.o

CGQVMOBJ = $(CGOBJ:%.o=$(BQ)/%.asm)
CGROBJ = $(CGOBJ:%.o=$(BR)/%.o) $(BR)/$(CGDIRNAME)/cg_syscalls.o
CGDOBJ = $(CGOBJ:%.o=$(BD)/%.o) $(BD)/$(CGDIRNAME)/cg_syscalls.o

UIQVMOBJ = $(UIOBJ:%.o=$(BQ)/%.asm)
UIROBJ = $(UIOBJ:%.o=$(BR)/%.o) $(BR)/$(UIDIRNAME)/ui_syscalls.o
UIDOBJ = $(UIOBJ:%.o=$(BD)/%.o) $(BD)/$(UIDIRNAME)/ui_syscalls.o



# --rules for the shared objects--
#release qagamei386.so
$(BR)/qagame$(ARCH).$(SHLIBEXT) : $(GROBJ)
	$(CC) $(RELEASE_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GROBJ)
  
#debug qagamei386.so
$(BD)/qagame$(ARCH).$(SHLIBEXT) : $(GDOBJ)
	$(CC) $(DEBUG_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GDOBJ)
  
#qvm qagame.qvm
$(BQ)/qagame.qvm : $(GQVMOBJ) $(BQ)/$(GDIRNAME)/bg_lib.asm
	$(Q3ASM) $(Q3ASM_FLAGS) -o $@ $(GQVMOBJ) $(GDIR)/g_syscalls.asm $(BQ)/$(GDIRNAME)/bg_lib.asm
  
  
#release cgamei386.so
$(BR)/cgame$(ARCH).$(SHLIBEXT) : $(CGROBJ)
	$(CC) $(RELEASE_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(CGROBJ)

#debug cgamei386.so
$(BD)/cgame$(ARCH).$(SHLIBEXT) : $(CGDOBJ)
	$(CC) $(DEBUG_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(CGDOBJ)

#qvm cgame.qvm
$(BQ)/cgame.qvm :	$(CGQVMOBJ) $(BQ)/$(GDIRNAME)/bg_lib.asm
	$(Q3ASM) $(Q3ASM_FLAGS) -o $@ $(CGQVMOBJ) $(CGDIR)/cg_syscalls.asm $(BQ)/$(GDIRNAME)/bg_lib.asm

  
#release uii386.so
$(BR)/ui$(ARCH).$(SHLIBEXT) : $(UIROBJ) 
	$(CC) $(RELEASE_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(UIROBJ)

#debug cgamei386.so
$(BD)/ui$(ARCH).$(SHLIBEXT) : $(UIDOBJ) 
	$(CC) $(DEBUG_CFLAGS) $(SHLIBLDFLAGS) -o $@ $(UIDOBJ)
  
#qvm ui.qvm
$(BQ)/ui.qvm: $(UIQVMOBJ) $(BQ)/$(GDIRNAME)/bg_lib.asm
	$(Q3ASM) $(Q3ASM_FLAGS) -o $@ $(UIQVMOBJ) $(UIDIR)/ui_syscalls.asm $(BQ)/$(GDIRNAME)/bg_lib.asm



# --rules for the objects--
#release g_*.o
$(BR)/$(GDIRNAME)/%.o: $(GDIR)/%.c 
	$(CC) $(RELEASE_CFLAGS) $(SLF_GAME_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<
  
#debug g_*.o
$(BD)/$(GDIRNAME)/%.o: $(GDIR)/%.c 
	$(CC) $(DEBUG_CFLAGS) $(SLF_GAME_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<
  
#qvm g_*.asm
$(BQ)/$(GDIRNAME)/%.asm: $(GDIR)/%.c 
	$(LCC) $(LCC_FLAGS) $(SLF_GAME_FLAGS) $(LCC_INCLUDES) -o $@ $<

#release cg_*.o
$(BR)/$(CGDIRNAME)/%.o: $(CGDIR)/%.c 
	$(CC) $(RELEASE_CFLAGS) $(SLF_CGAME_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<
  
#debug cg_*.o
$(BD)/$(CGDIRNAME)/%.o: $(CGDIR)/%.c 
	$(CC) $(DEBUG_CFLAGS) $(SLF_CGAME_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#qvm cg_*.asm
$(BQ)/$(CGDIRNAME)/%.asm: $(CGDIR)/%.c 
	$(LCC) $(LCC_FLAGS) $(SLF_CGAME_FLAGS) $(LCC_INCLUDES) -o $@ $<

#release ui_*.o
$(BR)/$(UIDIRNAME)/%.o: $(UIDIR)/%.c 
	$(CC) $(RELEASE_CFLAGS) $(SLF_UI_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<
  
#debug ui_*.o
$(BD)/$(UIDIRNAME)/%.o: $(UIDIR)/%.c 
	$(CC) $(DEBUG_CFLAGS) $(SLF_UI_FLAGS) $(SHLIBCFLAGS) -o $@ -c $<
  
#qvm ui_*.asm
$(BQ)/$(UIDIRNAME)/%.asm: $(UIDIR)/%.c 
	$(LCC) $(LCC_FLAGS) $(SLF_UI_FLAGS) $(LCC_INCLUDES) -o $@ $<


# --cleaning rules--
clean:clean-debug clean-release clean-qvm

clean-debug:
	rm -f $(BD)/$(GDIRNAME)/*.o
	rm -f $(BD)/$(CGDIRNAME)/*.o
	rm -f $(BD)/$(UIDIRNAME)/*.o

clean-release:
	rm -f $(BR)/$(GDIRNAME)/*.o
	rm -f $(BR)/$(CGDIRNAME)/*.o
	rm -f $(BR)/$(UIDIRNAME)/*.o

clean-qvm:
	rm -f $(BQ)/$(GDIRNAME)/*.asm
	rm -f $(BQ)/$(CGDIRNAME)/*.asm
	rm -f $(BQ)/$(UIDIRNAME)/*.asm



# --installing rules--
install-release:release
	@if [ ! -d $(Q3A_DIR) ];then mkdir $(Q3A_DIR);fi
	@if [ ! -d $(Q3A_DIR)/$(MOD_DIR) ];then mkdir $(Q3A_DIR)/$(MOD_DIR);fi
	cp $(BR)/*.so $(Q3A_DIR)/$(MOD_DIR)

install-debug:debug
	@if [ ! -d $(Q3A_DIR) ];then mkdir $(Q3A_DIR);fi
	@if [ ! -d $(Q3A_DIR)/$(MOD_DIR) ];then mkdir $(Q3A_DIR)/$(MOD_DIR);fi
	cp $(BD)/*.so $(Q3A_DIR)/$(MOD_DIR)

install-qvm:qvm
	@if [ ! -d $(Q3A_DIR) ];then mkdir $(Q3A_DIR);fi
	@if [ ! -d $(Q3A_DIR)/$(MOD_DIR) ];then mkdir $(Q3A_DIR)/$(MOD_DIR);fi
	@if [ ! -d $(Q3A_DIR)/$(MOD_DIR)/vm ];then mkdir $(Q3A_DIR)/$(MOD_DIR)/vm;fi
	cp $(BQ)/*.qvm $(Q3A_DIR)/$(MOD_DIR)/vm


# --dependency rules--
DEPEND_FILE=depend

depend:
	echo > $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(GDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BR)\/$(GDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(GDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BD)\/$(GDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(GDIR)/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*\.asm\)/$(BQ)\/$(GDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(CGDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BR)\/$(CGDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(CGDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BD)\/$(CGDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(CGDIR)/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*\.asm\)/$(BQ)\/$(CGDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(UIDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BR)\/$(UIDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(UIDIR)/*.c | $(SED) -e 's/^\(.*\.o\)/$(BD)\/$(UIDIRNAME)\/\1/g' >> $(DEPEND_FILE)
	$(CC) $(DEPEND_CFLAGS) $(LCC_INCLUDES) $(UIDIR)/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*\.asm\)/$(BQ)\/$(UIDIRNAME)\/\1/g' >> $(DEPEND_FILE)
        
include $(DEPEND_FILE) 

