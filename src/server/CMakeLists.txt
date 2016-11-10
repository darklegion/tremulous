
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
    -DPRODUCT_VERSION=""
    -DUSE_VOIP
    -DNDEBUG
    )

set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(APPLE)
set(APPLE_SOURCES ${PARENT_DIR}/sys/sys_osx.m)
endif(APPLE)

add_executable(
    tremded
    #
    server.h
    #
    sv_ccmds.c
    sv_client.c
    sv_game.c
    sv_init.c
    sv_main.c
    sv_net_chan.c
    sv_snapshot.c
    sv_world.c
    #
    ${PARENT_DIR}/qcommon/cm_load.c
    ${PARENT_DIR}/qcommon/cm_patch.c
    ${PARENT_DIR}/qcommon/cm_polylib.c
    ${PARENT_DIR}/qcommon/cm_test.c
    ${PARENT_DIR}/qcommon/cm_trace.c
    ${PARENT_DIR}/qcommon/cmd.cpp
    ${PARENT_DIR}/qcommon/common.c
    ${PARENT_DIR}/qcommon/crypto.c
    ${PARENT_DIR}/qcommon/cvar.cpp
    ${PARENT_DIR}/qcommon/files.c
    ${PARENT_DIR}/qcommon/huffman.c
    ${PARENT_DIR}/qcommon/ioapi.c
    ${PARENT_DIR}/qcommon/md4.c
    ${PARENT_DIR}/qcommon/msg.c
    ${PARENT_DIR}/qcommon/net_chan.c
    ${PARENT_DIR}/qcommon/net_ip.c
    ${PARENT_DIR}/qcommon/parse.c
    ${PARENT_DIR}/qcommon/q3_lauxlib.cpp
    ${PARENT_DIR}/qcommon/q_math.c
    ${PARENT_DIR}/qcommon/q_shared.c
    ${PARENT_DIR}/qcommon/unzip.c
    ${PARENT_DIR}/qcommon/vm.c
    ${PARENT_DIR}/qcommon/vm_interpreted.c
    ${PARENT_DIR}/qcommon/vm_x86.c
    #
    ${PARENT_DIR}/null/null_client.c
    ${PARENT_DIR}/null/null_input.c
    ${PARENT_DIR}/null/null_snddma.c
    #
    ${PARENT_DIR}/asm/ftola.c
    ${PARENT_DIR}/asm/snapvector.c
    #
    ${PARENT_DIR}/sys/con_log.c
    ${PARENT_DIR}/sys/con_tty.c
    ${PARENT_DIR}/sys/sys_main.cpp
    ${PARENT_DIR}/sys/sys_unix.c
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
