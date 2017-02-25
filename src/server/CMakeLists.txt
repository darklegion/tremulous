
#
##    .dMMMb  dMMMMMP dMMMMb  dMP dMP dMMMMMP dMMMMb
##   dMP" VP dMP     dMP.dMP dMP dMP dMP     dMP.dMP
##   VMMMb  dMMMP   dMMMMK" dMP dMP dMMMP   dMMMMK"
## dP .dMP dMP     dMP"AMF  YMvAP" dMP     dMP"AMF
## VMMMP" dMMMMMP dMP dMP    VP"  dMMMMMP dMP dMP
#

add_definitions(
    -DDEDICATED
    -DUSE_LOCAL_HEADERS
    -DPRODUCT_VERSION="1.2.0 pre-release"
    -DUSE_VOIP
    -DNDEBUG
    )

set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(APPLE)
set(APPLE_SOURCES ${PARENT_DIR}/sys/sys_osx.mm)
endif(APPLE)

add_executable(
    tremded
    #
    server.h
    #
    sv_ccmds.cpp
    sv_client.cpp
    sv_game.cpp
    sv_init.cpp
    sv_main.cpp
    sv_net_chan.cpp
    sv_snapshot.cpp
    sv_world.cpp
    #
    ${PARENT_DIR}/qcommon/cm_load.cpp
    ${PARENT_DIR}/qcommon/cm_patch.cpp
    ${PARENT_DIR}/qcommon/cm_polylib.cpp
    ${PARENT_DIR}/qcommon/cm_test.cpp
    ${PARENT_DIR}/qcommon/cm_trace.cpp
    ${PARENT_DIR}/qcommon/cmd.cpp
    ${PARENT_DIR}/qcommon/common.cpp
    ${PARENT_DIR}/qcommon/crypto.c
    ${PARENT_DIR}/qcommon/cvar.cpp
    ${PARENT_DIR}/qcommon/files.cpp
    ${PARENT_DIR}/qcommon/huffman.c
    ${PARENT_DIR}/qcommon/ioapi.c
    ${PARENT_DIR}/qcommon/md4.c
    ${PARENT_DIR}/qcommon/msg.cpp
    ${PARENT_DIR}/qcommon/net_chan.cpp
    ${PARENT_DIR}/qcommon/net_ip.cpp
    ${PARENT_DIR}/qcommon/parse.cpp
    ${PARENT_DIR}/qcommon/q3_lauxlib.cpp
    ${PARENT_DIR}/qcommon/q_math.c
    ${PARENT_DIR}/qcommon/q_shared.c
    ${PARENT_DIR}/qcommon/unzip.c
    ${PARENT_DIR}/qcommon/vm.cpp
    ${PARENT_DIR}/qcommon/vm_interpreted.cpp
    ${PARENT_DIR}/qcommon/vm_x86.cpp
    #
    ${PARENT_DIR}/null/null_client.cpp
    ${PARENT_DIR}/null/null_input.cpp
    ${PARENT_DIR}/null/null_snddma.cpp
    #
    ${PARENT_DIR}/asm/ftola.c
    ${PARENT_DIR}/asm/snapvector.c
    #
    ${PARENT_DIR}/sys/con_log.cpp
    ${PARENT_DIR}/sys/con_tty.cpp
    ${PARENT_DIR}/sys/sys_main.cpp
    ${PARENT_DIR}/sys/sys_unix.cpp
    ${APPLE_SOURCES}
    #
    ${PARENT_DIR}/zlib/adler32.c
    ${PARENT_DIR}/zlib/crc32.c
    ${PARENT_DIR}/zlib/inffast.c
    ${PARENT_DIR}/zlib/inflate.c
    ${PARENT_DIR}/zlib/inftrees.c
    ${PARENT_DIR}/zlib/zutil.c
    )

if(APPLE)
 # FIXME Prefixed with "lua" to prevent cmake from doing "-l-framework Cocoa"
 set(FRAMEWORKS "-framework Cocoa -framework Security -framework OpenAL -framework IOKit")
else(APPLE)
 if(UNIX)
  set(SYSLIBS dl rt)
 endif(UNIX)
endif(APPLE)

target_link_libraries(
    tremded
    #
    lua
    script_api
    nettle
    zlib
    ${FRAMEWORKS}
    ${SYSLIBS}
    )
    
include_directories(
    ${PARENT_DIR}/script
    ${PARENT_DIR}/lua-5.3.3/include
    ${PARENT_DIR}/sol
    ${PARENT_DIR}/script/rapidjson
    ${PARENT_DIR}/nettle-3.3
    ${PARENT_DIR}/zlib
    )
