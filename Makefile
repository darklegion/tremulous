# --
# -- Makefile for building Q3A qvms on Linux
# -- 
# -- Copyleft under the GPL by Yumpee
# -- yumpee@freeshell.org
# -- http://freeshell.org/~yumpee
# -- with additions/modifications by Timbo
# -- tma@dcs.ed.ac.uk


# -- Default definitions, can be over-ridden by commandline options
CPP=/usr/games/quake3/tools/q3cpp
CPP_FLAGS=-DQ3_VM
INCLUDES=-Icgame -Igame -Iui

RCC=/usr/games/quake3/tools/q3rcc
RCC_FLAGS=-target=bytecode -g

Q3ASM=/usr/games/quake3/tools/q3asm
Q3ASM_FLAGS=

CC=gcc
CC_DEPEND_FLAGS=-MM

SED=sed

Q3ADIR=/usr/games/quake3
MOD=tremulous


# -- Object files of each QVM
VM_CGAME_OBJ=    cgame/vm/cg_main.asm		\
                 cgame/vm/cg_mem.asm	\
                 cgame/vm/cg_consolecmds.asm	\
                 cgame/vm/cg_draw.asm		\
                 cgame/vm/cg_drawtools.asm	\
                 cgame/vm/cg_effects.asm	\
                 cgame/vm/cg_ents.asm		\
                 cgame/vm/cg_event.asm		\
                 cgame/vm/cg_info.asm		\
                 cgame/vm/cg_localents.asm	\
	               cgame/vm/cg_marks.asm		\
                 cgame/vm/cg_players.asm	\
                 cgame/vm/cg_playerstate.asm	\
                 cgame/vm/cg_predict.asm	\
                 cgame/vm/cg_scoreboard.asm	\
                 cgame/vm/cg_servercmds.asm	\
                 cgame/vm/cg_snapshot.asm	\
                 cgame/vm/cg_view.asm		\
                 cgame/vm/cg_weapons.asm	\
                 cgame/vm/cg_creep.asm	\
                 cgame/vm/cg_lightmap.asm \
                 cgame/vm/cg_scanner.asm \
                 game/vm/bg_misc.asm		\
                 game/vm/bg_pmove.asm		\
                 game/vm/bg_slidemove.asm	\
                 game/vm/bg_lib.asm		\
                 game/vm/q_math.asm		\
                 game/vm/q_shared.asm


VM_GAME_OBJ=    game/vm/g_main.asm		\
                game/vm/bg_misc.asm		\
                game/vm/bg_lib.asm		\
                game/vm/bg_pmove.asm		\
                game/vm/bg_slidemove.asm	\
                game/vm/q_math.asm		\
                game/vm/q_shared.asm		\
                game/vm/g_active.asm		\
                game/vm/g_arenas.asm		\
                game/vm/g_client.asm		\
                game/vm/g_cmds.asm		\
                game/vm/g_combat.asm		\
                game/vm/g_items.asm		\
                game/vm/g_buildable.asm		\
                game/vm/g_creep.asm		\
                game/vm/g_mem.asm		\
                game/vm/g_misc.asm		\
                game/vm/g_missile.asm		\
                game/vm/g_mover.asm		\
                game/vm/g_session.asm		\
                game/vm/g_spawn.asm		\
                game/vm/g_svcmds.asm		\
                game/vm/g_target.asm		\
                game/vm/g_team.asm		\
                game/vm/g_trigger.asm		\
                game/vm/g_utils.asm		\
                game/vm/g_weapon.asm

VM_UI_OBJ=	ui/vm/ui_main.asm		\
            ui/vm/ui_mods.asm		\
            ui/vm/ui_gameinfo.asm		\
            ui/vm/ui_atoms.asm		\
            ui/vm/ui_cinematics.asm		\
            ui/vm/ui_connect.asm		\
            ui/vm/ui_controls2.asm		\
            ui/vm/ui_demo2.asm		\
            ui/vm/ui_mfield.asm		\
            ui/vm/ui_credits.asm		\
            ui/vm/ui_menu.asm		\
            ui/vm/ui_ingame.asm		\
            ui/vm/ui_confirm.asm		\
            ui/vm/ui_setup.asm		\
            ui/vm/ui_options.asm		\
            ui/vm/ui_display.asm		\
            ui/vm/ui_sound.asm		\
            ui/vm/ui_network.asm		\
            ui/vm/ui_playermodel.asm	\
            ui/vm/ui_players.asm		\
            ui/vm/ui_playersettings.asm	\
            ui/vm/ui_preferences.asm	\
            ui/vm/ui_qmenu.asm		\
            ui/vm/ui_serverinfo.asm		\
            ui/vm/ui_servers2.asm		\
            ui/vm/ui_sparena.asm		\
            ui/vm/ui_specifyserver.asm	\
            ui/vm/ui_sppostgame.asm		\
            ui/vm/ui_splevel.asm		\
            ui/vm/ui_spskill.asm		\
            ui/vm/ui_startserver.asm	\
            ui/vm/ui_team.asm		\
            ui/vm/ui_video.asm		\
            ui/vm/ui_addbots.asm		\
            ui/vm/ui_removebots.asm		\
            ui/vm/ui_teamorders.asm		\
            ui/vm/ui_dynamicmenu.asm		\
            ui/vm/ui_loadconfig.asm		\
            ui/vm/ui_saveconfig.asm		\
            ui/vm/ui_cdkey.asm		\
            game/vm/bg_misc.asm		\
            game/vm/bg_lib.asm		\
            game/vm/q_math.asm		\
            game/vm/q_shared.asm


# -- Makefile's main targets
all: cgame game ui

install: cgame game ui
	-mkdir -p $(Q3ADIR)/$(MOD)/vm
	-cp cgame/vm/cgame.qvm $(Q3ADIR)/$(MOD)/vm 
	-cp game/vm/qagame.qvm $(Q3ADIR)/$(MOD)/vm 
	-cp ui/vm/ui.qvm $(Q3ADIR)/$(MOD)/vm 

cgame: cgame/vm/cgame.qvm

game: game/vm/qagame.qvm

ui: ui/vm/ui.qvm


# -- Rules for compiling each QVM
cgame/vm/cgame.qvm: $(VM_CGAME_OBJ)
	 $(Q3ASM) $(Q3ASM_FLAGS) $^ cgame/cg_syscalls.asm
	-mv q3asm.qvm cgame/vm/cgame.qvm
	-mv q3asm.map cgame/vm/cgame.map

game/vm/qagame.qvm: $(VM_GAME_OBJ)
	 $(Q3ASM) $(Q3ASM_FLAGS) $^ game/g_syscalls.asm
	-mv q3asm.qvm game/vm/qagame.qvm
	-mv q3asm.map game/vm/qagame.map

ui/vm/ui.qvm: $(VM_UI_OBJ)
	 $(Q3ASM) $(Q3ASM_FLAGS) $^ ui/ui_syscalls.asm
	-mv q3asm.qvm ui/vm/ui.qvm
	-mv q3asm.map ui/vm/ui.map


# -- Rules for compiling single files
cgame/vm/%.asm: cgame/%.c
	$(CPP) $(CPP_FLAGS) $(INCLUDES) $< cgame/vm/$*.i
	$(RCC) $(RCC_FLAGS) cgame/vm/$*.i $@
	-rm cgame/vm/$*.i

game/vm/%.asm: game/%.c
	$(CPP) $(CPP_FLAGS) $(INCLUDES) $< game/vm/$*.i
	$(RCC) $(RCC_FLAGS) game/vm/$*.i $@
	-rm game/vm/$*.i

ui/vm/%.asm: ui/%.c
	$(CPP) $(CPP_FLAGS) $(INCLUDES) $< ui/vm/$*.i
	$(RCC) $(RCC_FLAGS) ui/vm/$*.i $@
	-rm ui/vm/$*.i


# -- Dependency rules (made automatically in file Q3A.depend)
depend:
	-echo > Q3A.depend
	-$(CC) $(CC_DEPEND_FLAGS) -Igame -Icgame -Iui cgame/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*asm\)/cgame\/vm\/\1/g' >> Q3A.depend
	-$(CC) $(CC_DEPEND_FLAGS) -Igame -Icgame -Iui game/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*asm\)/game\/vm\/\1/g' >> Q3A.depend
	-$(CC) $(CC_DEPEND_FLAGS) -Igame -Icgame -Iui ui/*.c | $(SED) -e 's/\.o/.asm/g' -e 's/^\(.*asm\)/ui\/vm\/\1/g' >> Q3A.depend

include Q3A.depend


# -- Clean up object files
clean: clean_cgame clean_game clean_ui

clean_cgame:
	-rm $(VM_CGAME_OBJ)
	-rm cgame/cm/cgame.qvm

clean_game:
	-rm $(VM_GAME_OBJ)
	-rm game/vm/qagame.qvm

clean_ui:
	-rm $(VM_UI_OBJ)
	-rm ui/vm/ui.qvm

# -- EOF
