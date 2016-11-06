
#   M""""""""M                                       dP                             #
#   Mmmm  mmmM                                       88                             #
#   MMMM  MMMM 88d888b. .d8888b. 88d8b.d8b. dP    dP 88 .d8888b. dP    dP .d8888b.  #
#   MMMM  MMMM 88'  `88 88ooood8 88'`88'`88 88    88 88 88'  `88 88    88 Y8ooooo.  #
#   MMMM  MMMM 88       88.  ... 88  88  88 88.  .88 88 88.  .88 88.  .88       88  #
#   MMMM  MMMM dP       `88888P' dP  dP  dP `88888P' dP `88888P' `88888P' `88888P'  #
#   MMMMMMMMMM                                                                      #


cmake_minimum_required(VERSION 3.5)
project(Tremulous C CXX)
#set(CMAKE_MODULE_PATH cmake)

set(CMAKE_CXX_FLAGS -std=c++1y)

option(USE_INTERNAL_JPEG "" ON)
option(USE_OPENAL "" ON)
option(USE_OPENAL_DLOPEN "" ON)
option(USE_VOIP "" ON)
option(USE_MUMBLE "" ON)
option(USE_CODEC_OPUS "" ON)
option(USE_RENDERER_DLOPEN "" ON)
option(USE_RESTCLIENT "" ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindQVM.cmake)

find_package(CURL)
find_package(OpenGL)
find_package(OpenAL)

set(SDL2_PREFIX "/usr/local")
set(SDL2_LIBRARIES "-L${SDL2_PREFIX} -lSDL2")
set(SDL2_INCLUDE_DIRS "${SDL2_PREFIX}/inlcude/SDL2")

# builtins 
add_subdirectory(src/lua-5.3.3)
add_subdirectory(src/nettle-3.3)
add_subdirectory(src/zlib)
add_subdirectory(src/jpeg-8c)

# audio
add_subdirectory(src/libspeex)
add_subdirectory(src/opus-1.1)
add_subdirectory(src/opusfile-0.5)
add_subdirectory(src/libogg-1.3.1)
add_subdirectory(src/libvorbis-1.3.4)

# engine
add_subdirectory(src/script)
add_subdirectory(src/renderercommon)
add_subdirectory(src/renderergl1)
add_subdirectory(src/renderergl2)
add_subdirectory(src/restclient)

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

