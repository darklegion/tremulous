#
##  _   _ ___    ____          _      
## | | | |_ _|  / ___|___   __| | ___ 
## | | | || |  | |   / _ \ / _` |/ _ \
## | |_| || |  | |__| (_) | (_| |  __/
##  \___/|___|  \____\___/ \__,_|\___|
##                                    
#

set(CMAKE_INSTALL_NAME_DIR ${PROJECT_BINARY_DIR}/gpp)

set(BG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../game)
set(QC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../qcommon)
set(RC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../renderercommon)
set(CLIENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../client)

add_definitions( -DUI )

add_library(
    ui SHARED 
    ui_atoms.c
    ui_gameinfo.c
    ui_local.h
    ui_main.c
    ui_shared.c
    ui_shared.h
    ui_syscalls.c
    #
    ${BG_SOURCE_DIR}/bg_lib.h
    ${BG_SOURCE_DIR}/bg_public.h
    ${BG_SOURCE_DIR}/bg_alloc.c
    ${BG_SOURCE_DIR}/bg_lib.c
    ${BG_SOURCE_DIR}/bg_misc.c
    ${BG_SOURCE_DIR}/bg_voice.c
    #
    ${QC_SOURCE_DIR}/q_shared.h
    ${QC_SOURCE_DIR}/q_shared.c
    ${QC_SOURCE_DIR}/q_math.c
    #
    ${RC_SOURCE_DIR}/tr_types.h
    #
    ${CLIENT_SOURCE_DIR}/keycodes.h
    )

include_directories(
    ${QC_SOURCE_DIR}
    ${BG_SOURCE_DIR}
    )
