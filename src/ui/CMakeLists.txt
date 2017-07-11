#
##  _   _ ___    ____          _      
## | | | |_ _|  / ___|___   __| | ___ 
## | | | || |  | |   / _ \ / _` |/ _ \
## | |_| || |  | |__| (_) | (_| |  __/
##  \___/|___|  \____\___/ \__,_|\___|
##                                    
#

set(CMAKE_INSTALL_NAME_DIR ${PROJECT_BINARY_DIR}/gpp)

set(BG_SOURCE_DIR ../game)
set(QC_SOURCE_DIR ../qcommon)
set(RC_SOURCE_DIR ../renderercommon)
set(CLIENT_SOURCE_DIR ../client)

add_definitions( -DUI )

#add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/menudef.h
#    COMMAND ${CMAKE_COMMAND}
#    ARGS -E copy ${CMAKE_BINARY_DIR}/assets/ui/menudef.h ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/menudef.h
#    DEPENDS ${CMAKE_BINARY_DIR}/assets/ui/menudef.h
#    )
#set_source_files_properties(menudef.h PROPERTIES GENERATED TRUE)

set(UI_SOURCES
    ui_main.c # Must be listed first!
    ui_atoms.c
    ui_gameinfo.c
    ui_local.h
    ui_shared.c
    ui_shared.h
    ${CMAKE_SOURCE_DIR}/assets/ui/menudef.h
    ${BG_SOURCE_DIR}/bg_lib.h
    ${BG_SOURCE_DIR}/bg_public.h
    ${BG_SOURCE_DIR}/bg_alloc.c
    ${BG_SOURCE_DIR}/bg_lib.c
    ${BG_SOURCE_DIR}/bg_misc.c
    ${BG_SOURCE_DIR}/bg_voice.c
    ${QC_SOURCE_DIR}/q_shared.h
    ${QC_SOURCE_DIR}/q_shared.c
    ${QC_SOURCE_DIR}/q_math.c
    ${RC_SOURCE_DIR}/tr_types.h
    ${CLIENT_SOURCE_DIR}/keycodes.h
    )

add_library(
    ui SHARED
    ${UI_SOURCES}
    ui_syscalls.c
    )

target_include_directories(
    ui PUBLIC
    ${QC_SOURCE_DIR}
    ${BG_SOURCE_DIR}
    )

include(${CMAKE_SOURCE_DIR}/cmake/AddQVM.cmake)
add_qvm( ui ${UI_SOURCES} ui_syscalls.asm )

add_custom_command(
    TARGET ui POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E copy ${CMAKE_CURRENT_BINARY_DIR}/libui${CMAKE_SHARED_LIBRARY_SUFFIX} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gpp/ui${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
