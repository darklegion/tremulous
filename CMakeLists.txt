
#   M""""""""M                                       dP                             #
#   Mmmm  mmmM                                       88                             #
#   MMMM  MMMM 88d888b. .d8888b. 88d8b.d8b. dP    dP 88 .d8888b. dP    dP .d8888b.  #
#   MMMM  MMMM 88'  `88 88ooood8 88'`88'`88 88    88 88 88'  `88 88    88 Y8ooooo.  #
#   MMMM  MMMM 88       88.  ... 88  88  88 88.  .88 88 88.  .88 88.  .88       88  #
#   MMMM  MMMM dP       `88888P' dP  dP  dP `88888P' dP `88888P' `88888P' `88888P'  #
#   MMMMMMMMMM                                                                      #


cmake_minimum_required(VERSION 3.5)
project(Tremulous C CXX)

set(CMAKE_CXX_FLAGS -std=c++1y)
set(CMAKE_C_FLAGS -std=gnu99)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
endif()

# XXX This one doesn't work yet
option(USE_RENDERER_DLOPEN "" ON)

# These are not used at the moment 
if (APPLE OR WIN32)
  option(USE_INTERNAL_SDL2 "" ON)
else ()
  option(USE_INTERNAL_SDL2 "" OFF)
endif()

option(USE_INTERNAL_JPEG "" ON)
option(USE_OPENAL "" ON)
option(USE_OPENAL_DLOPEN "" ON)
option(USE_VOIP "" OFF)
option(USE_MUMBLE "" OFF)
option(USE_CODEC_OPUS "" ON)
option(USE_RESTCLIENT "" ON)

include(${CMAKE_SOURCE_DIR}/cmake/build_dir)
include(${CMAKE_SOURCE_DIR}/cmake/debug_cflags)

# builtins 
add_subdirectory(external/SDL2)
add_subdirectory(external/lua-5.3.3)
add_subdirectory(external/nettle-3.3)
add_subdirectory(external/zlib)
add_subdirectory(external/jpeg-8c)
add_subdirectory(external/restclient)

# audio
add_subdirectory(external/opus-1.1.4)
add_subdirectory(external/opusfile-0.8)
add_subdirectory(external/libogg-1.3.2)
add_subdirectory(external/libvorbis-1.3.5)

# engine
add_subdirectory(src/script)
add_subdirectory(src/renderercommon)
add_subdirectory(src/renderergl1)
add_subdirectory(src/renderergl2)

#         .                    .
#   ___   |   `   ___  , __   _/_
# .'   `  |   | .'   ` |'  `.  |
# |       |   | |----' |    |  |
#  `._.' /\__ / `.___, /    |  \__/
#

add_subdirectory(src/client)

#   ____   ___  .___  _   __   ___  .___
#  (     .'   ` /   \ |   /  .'   ` /   \
#  `--.  |----' |   ' `  /   |----' |   '
# \___.' `.___, /      \/    `.___, /
#
 
add_subdirectory(src/server)

#   ___
# .'   \  .___    ___  , __     ___.   ___  .___
# |       /   \  /   ` |'  `. .'   ` .'   ` /   \
# |    _  |   ' |    | |    | |    | |----' |   '
#  `.___| /     `.__/| /    |  `---| `.___, /
#                              \___/

if(BUILD_GRANGER)
  add_subdirectory(src/granger/src)
endif(BUILD_GRANGER)

#  _______ ___ ___ _______                              __ __
# |       |   |   |   |   | .----.-----.--------.-----.|__|  |.-----.----.
# |   -  _|   |   |       | |  __|  _  |        |  _  ||  |  ||  -__|   _|
# |_______|\_____/|__|_|__| |____|_____|__|__|__|   __||__|__||_____|__|
#                                               |__|
# TODO: DO NOT build unless building a QVM

add_subdirectory(src/tools)

#
# ,   . `
# |   | |
# |   | |
# `._/| /
#

add_subdirectory(src/ui)

#   ___.   ___  , _ , _     ___
# .'   `  /   ` |' `|' `. .'   `
# |    | |    | |   |   | |----'
#  `---| `.__/| /   '   / `.___,
#  \___/
#

add_subdirectory(src/game)

#   ___    ___.   ___  , _ , _     ___
# .'   ` .'   `  /   ` |' `|' `. .'   `
# |      |    | |    | |   |   | |----'
#  `._.'  `---| `.__/| /   '   / `.___,
#         \___/
#

add_subdirectory(src/cgame)

#            _____)                          ____  ___)
#          /                                (, /   /      /)
#         /   ___   __  _  __   _    _  __    /---/      (/_     _  ______
#        /     / ) / (_(_(_/ (_(_/__(/_/ (_) /   (__(_(_/_)  o  (__(_) // (_
#       (____ /               .-/         (_/
#                            (_/

# Author: Victor Roemer  wtfbbqhax, <victor@badsec.org>. 
#  Date: Sat Nov  5 06:49:08 2016 

