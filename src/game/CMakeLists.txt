#
##    ____                                      _      
##   / ___| __ _ _ __ ___   ___    ___ ___   __| | ___ 
##  | |  _ / _` | '_ ` _ \ / _ \  / __/ _ \ / _` |/ _ \
##  | |_| | (_| | | | | | |  __/ | (_| (_) | (_| |  __/
##   \____|\__,_|_| |_| |_|\___|  \___\___/ \__,_|\___|
##                                                     
#

set(CMAKE_INSTALL_NAME_DIR ${PROJECT_BINARY_DIR}/gpp)

set(QC_SOURCE_DIR ../qcommon)
#set(RC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../renderercommon)
#set(CLIENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../client)

add_definitions( -DGAME )

set( GAME_SOURCES
    g_main.c # Must be listed first!
    bg_alloc.c
    bg_lib.c
    bg_lib.h
    bg_local.h
    bg_misc.c
    bg_pmove.c
    bg_public.h
    bg_slidemove.c
    bg_voice.c
    g_active.c
    g_admin.c
    g_admin.h
    g_buildable.c
    g_client.c
    g_cmds.c
    g_combat.c
    g_local.h
    g_maprotation.c
    g_misc.c
    g_missile.c
    g_mover.c
    g_namelog.c
    g_physics.c
    g_playermodel.c
    g_public.h
    g_session.c
    g_spawn.c
    g_svcmds.c
    g_target.c
    g_team.c
    g_trigger.c
    g_utils.c
    g_weapon.c
    g_weapondrop.c
    tremulous.h
    ${QC_SOURCE_DIR}/q_shared.h 
    ${QC_SOURCE_DIR}/q_shared.c
    ${QC_SOURCE_DIR}/q_math.c
    )

add_library( game SHARED ${GAME_SOURCES} g_syscalls.c )

include( ${CMAKE_SOURCE_DIR}/cmake/AddQVM.cmake )
add_qvm( game ${GAME_SOURCES} g_syscalls.asm )

add_custom_command(
    TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${CMAKE_CURRENT_BINARY_DIR}/libgame${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gpp/game${CMAKE_SHARED_LIBRARY_SUFFIX}
    )

