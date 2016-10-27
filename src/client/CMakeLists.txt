
#
##  .o88b. db      d888888b d88888b d8b   db d888888b
## d8P  Y8 88        `88'   88'     888o  88 `~~88~~'
## 8P      88         88    88ooooo 88V8o 88    88
## 8b      88         88    88~~~~~ 88 V8o88    88
## Y8b  d8 88booo.   .88.   88.     88  V888    88
##  `Y88P' Y88888P Y888888P Y88888P VP   V8P    YP
#
if(APPLE)
 set(SDL2_PREFIX "/usr/local")
 set(SDL2_LIBRARIES "-L${SDL2_PREFIX} -lSDL2")
 set(SDL2_INCLUDE_DIRS "${SDL2_PREFIX}/include/SDL2")
else(APPLE)
 set(SDL2_PREFIX "/usr")
 set(SDL2_LIBRARIES "-L${SDL2_PREFIX} -lSDL2")
 set(SDL2_INCLUDE_DIRS "${SDL2_PREFIX}/include/SDL2")
endif(APPLE)

add_definitions(
    -DUSE_LOCAL_HEADERS
    -DUSE_RESTCLIENT
    -DUSE_VOIP
    -DUSE_MUMBLE
    -DUSE_ALLOCA # opus
    -DUSE_CODEC_OPUS
    -DUSE_OPENAL
    -DUSE_OPENAL_DLOPEN
    -DUSE_INTERNAL_JPEG
    #FIXME-DUSE_RENDERER_DLOPEN
    #-DHAVE_LRINTF
    -DNDEBUG
    -DPRODUCT_VERSION=""
    )

set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(APPLE)
set(APPLE_SOURCES ${PARENT_DIR}/sys/sys_osx.m)
endif(APPLE)

add_executable(
    tremulous
    #
    ${PARENT_DIR}/asm/ftola.c
    ${PARENT_DIR}/asm/snapvector.c
    #
    cl_avi.c
    cl_cgame.c
    cl_cin.c
    cl_console.c
    cl_curl.c
    cl_input.c
    cl_keys.c
    cl_main.c
    cl_net_chan.c
    cl_parse.c
    cl_rest.cpp
    cl_scrn.c
    cl_ui.c
    cl_updates.cpp
    libmumblelink.c
    qal.c
    snd_adpcm.c
    snd_codec.c
    snd_codec_ogg.c
    snd_codec_opus.c
    snd_codec_wav.c
    snd_dma.c
    snd_main.c
    snd_mem.c
    snd_mix.c
    snd_openal.c
    snd_wavelet.c
    #
    #${PARENT_DIR}/restclient/connection.cpp
    #${PARENT_DIR}/restclient/connection.h
    #${PARENT_DIR}/restclient/helpers.cpp
    #${PARENT_DIR}/restclient/helpers.h
    #${PARENT_DIR}/restclient/restclient.cpp
    #${PARENT_DIR}/restclient/restclient.h
    #${PARENT_DIR}/restclient/version.h
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
    ${PARENT_DIR}/qcommon/md5.c
    ${PARENT_DIR}/qcommon/msg.c
    ${PARENT_DIR}/qcommon/net_chan.c
    ${PARENT_DIR}/qcommon/net_ip.c
    ${PARENT_DIR}/qcommon/parse.c
    ${PARENT_DIR}/qcommon/puff.c
    ${PARENT_DIR}/qcommon/q_shared.c
    ${PARENT_DIR}/qcommon/q3_lauxlib.cpp
    ${PARENT_DIR}/qcommon/q_math.c
    ${PARENT_DIR}/qcommon/unzip.c
    ${PARENT_DIR}/qcommon/vm.c
    ${PARENT_DIR}/qcommon/vm_interpreted.c
    ${PARENT_DIR}/qcommon/vm_x86.c
    #
    ${PARENT_DIR}/sdl/sdl_input.c
    ${PARENT_DIR}/sdl/sdl_snd.c
    #
    ${PARENT_DIR}/server/sv_ccmds.c
    ${PARENT_DIR}/server/sv_client.c
    ${PARENT_DIR}/server/sv_game.c
    ${PARENT_DIR}/server/sv_init.c
    ${PARENT_DIR}/server/sv_main.c
    ${PARENT_DIR}/server/sv_net_chan.c
    ${PARENT_DIR}/server/sv_snapshot.c
    ${PARENT_DIR}/server/sv_world.c
    #
    ${PARENT_DIR}/sys/con_log.c
    ${PARENT_DIR}/sys/con_tty.c
    ${PARENT_DIR}/sys/lnettlelib.c
    ${PARENT_DIR}/sys/sys_main.cpp
    ${PARENT_DIR}/sys/sys_unix.c
    ${APPLE_SOURCES}
    )

if(APPLE)
 # FIXME Prefixed with "lua" to prevent cmake from doing "-l-framework Cocoa"
 set(FRAMEWORKS "lua -framework Cocoa -framework Security -framework OpenAL -framework IOKit")
else(APPLE)
 if(UNIX)
  set(SYSLIBS dl rt)
 endif(UNIX)
endif(APPLE)

target_link_libraries(
    tremulous
    #
    lua
    nettle
    zlib
    ogg
    opus
    opusfile
    #
    script_rapidjson
    restclient
    renderergl1
    #renderergl2
    #
    ${FRAMEWORKS}
    ${CURL_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${SYSLIBS}
    )

include_directories(
    ${PARENT_DIR}
    ${PARENT_DIR}/zlib
    ${PARENT_DIR}/restclient
    ${PARENT_DIR}/rapidjson
    ${PARENT_DIR}/SDL2/include
    ${PARENT_DIR}/jpeg-8c
    ${PARENT_DIR}/libogg-1.3.1/include
    ${PARENT_DIR}/lua-5.3.3/include
    ${PARENT_DIR}/sol
    ${PARENT_DIR}/nettle-3.3
    ${PARENT_DIR}/opus-1.1/include
    ${PARENT_DIR}/opusfile-0.5/include
    ${SDL2_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    )

