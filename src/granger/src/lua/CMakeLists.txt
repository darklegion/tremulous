set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

add_library( granger_lua STATIC
    ${PARENT_DIR}/lua-5.3.3/src/lapi.c
    ${PARENT_DIR}/lua-5.3.3/src/lcode.c
    ${PARENT_DIR}/lua-5.3.3/src/lctype.c
    ${PARENT_DIR}/lua-5.3.3/src/ldebug.c
    ${PARENT_DIR}/lua-5.3.3/src/ldo.c
    ${PARENT_DIR}/lua-5.3.3/src/ldump.c
    ${PARENT_DIR}/lua-5.3.3/src/lfunc.c
    ${PARENT_DIR}/lua-5.3.3/src/lgc.c
    ${PARENT_DIR}/lua-5.3.3/src/llex.c
    ${PARENT_DIR}/lua-5.3.3/src/lmem.c
    ${PARENT_DIR}/lua-5.3.3/src/lobject.c
    ${PARENT_DIR}/lua-5.3.3/src/lopcodes.c
    ${PARENT_DIR}/lua-5.3.3/src/lparser.c
    ${PARENT_DIR}/lua-5.3.3/src/lstate.c
    ${PARENT_DIR}/lua-5.3.3/src/lstring.c
    ${PARENT_DIR}/lua-5.3.3/src/ltable.c
    ${PARENT_DIR}/lua-5.3.3/src/ltm.c
    ${PARENT_DIR}/lua-5.3.3/src/lundump.c
    ${PARENT_DIR}/lua-5.3.3/src/lvm.c
    ${PARENT_DIR}/lua-5.3.3/src/lzio.c
    ${PARENT_DIR}/lua-5.3.3/src/lauxlib.c
    ${PARENT_DIR}/lua-5.3.3/src/lbaselib.c
    ${PARENT_DIR}/lua-5.3.3/src/lbitlib.c
    ${PARENT_DIR}/lua-5.3.3/src/lcorolib.c
    ${PARENT_DIR}/lua-5.3.3/src/ldblib.c
    ${PARENT_DIR}/lua-5.3.3/src/liolib.c
    ${PARENT_DIR}/lua-5.3.3/src/lmathlib.c
    ${PARENT_DIR}/lua-5.3.3/src/loslib.c
    ${PARENT_DIR}/lua-5.3.3/src/lstrlib.c
    ${PARENT_DIR}/lua-5.3.3/src/ltablib.c
    ${PARENT_DIR}/lua-5.3.3/src/lutf8lib.c
    ${PARENT_DIR}/lua-5.3.3/src/loadlib.c
    ${PARENT_DIR}/lua-5.3.3/src/linit.c
    )

if(APPLE)
  add_definitions(-DLUA_USE_MACOSX)
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-DLUA_USE_LINUX)
endif()

add_definitions (
    -DLUA_COMPAT_5_2
    -DNDEBUG
    -mfpmath=sse
    -ffast-math
    -DGRANGER
    )

include_directories (
    include
    )
