#
##  ____                                           _      
## / ___|__ _  __ _ _ __ ___   ___    ___ ___   __| | ___ 
##| |   / _` |/ _` | '_ ` _ \ / _ \  / __/ _ \ / _` |/ _ \
##| |__| (_| | (_| | | | | | |  __/ | (_| (_) | (_| |  __/
## \____\__, |\__,_|_| |_| |_|\___|  \___\___/ \__,_|\___|
##      |___/                                             
#

set(CMAKE_INSTALL_NAME_DIR ${PROJECT_BINARY_DIR}/gpp)

set(BG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../game)
set(QC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../qcommon)
set(UI_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../ui)
set(RC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../renderercommon)

add_definitions( -DCGAME )

add_library(
    cgame SHARED
    #
    cg_public.h
    cg_local.h
    binaryshader.h
    #
    cg_animation.c
    cg_animmapobj.c
    cg_attachment.c
    cg_buildable.c
    cg_consolecmds.c
    cg_draw.c
    cg_drawtools.c
    cg_ents.c
    cg_event.c
    cg_main.c
    cg_marks.c
    cg_particles.c
    cg_players.c
    cg_playerstate.c
    cg_predict.c
    cg_rangemarker.c
    cg_scanner.c
    cg_servercmds.c
    cg_snapshot.c
    cg_syscalls.c
    cg_trails.c
    cg_tutorial.c
    cg_view.c
    cg_weapons.c
    #
    ${UI_SOURCE_DIR}/ui_shared.h
    ${UI_SOURCE_DIR}/ui_shared.c
    #
    ${BG_SOURCE_DIR}/bg_lib.h
    ${BG_SOURCE_DIR}/bg_public.h
    ${BG_SOURCE_DIR}/bg_alloc.c
    ${BG_SOURCE_DIR}/bg_lib.c
    ${BG_SOURCE_DIR}/bg_misc.c
    ${BG_SOURCE_DIR}/bg_voice.c
    ${BG_SOURCE_DIR}/bg_pmove.c
    ${BG_SOURCE_DIR}/bg_slidemove.c
    #
    ${QC_SOURCE_DIR}/q_shared.h
    ${QC_SOURCE_DIR}/q_shared.c
    ${QC_SOURCE_DIR}/q_math.c
    #
    ${RC_SOURCE_DIR}/tr_types.h
    )

include_directories(
    ${BG_SOURCE_DIR}
    ${QC_SOURCE_DIR}
    ${RC_SOURCE_DIR}
    ${UI_SOURCE_DIR}
    )
