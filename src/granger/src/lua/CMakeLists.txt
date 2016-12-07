add_definitions(-DLUA_COMPAT_5_2)

if(NOT WIN32)
    add_definitions(-Wall -Wextra)
endif(NOT WIN32)

if(APPLE)
    add_definitions(-DLUA_USE_MACOSX)
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DLUA_USE_LINUX)
endif()

add_library(granger_lua
    lapi.c
    lapi.h
    lauxlib.c
    lauxlib.h
    lbaselib.c
    lbitlib.c
    lcode.c
    lcode.h
    lcorolib.c
    lctype.c
    lctype.h
    ldblib.c
    ldebug.c
    ldebug.h
    ldo.c
    ldo.h
    ldump.c
    lfunc.c
    lfunc.h
    lgc.c
    lgc.h
    linit.c
    liolib.c
    llex.c
    llex.h
    llimits.h
    lmathlib.c
    lmem.c
    lmem.h
    loadlib.c
    lobject.c
    lobject.h
    lopcodes.c
    lopcodes.h
    loslib.c
    lparser.c
    lparser.h
    lprefix.h
    lstate.c
    lstate.h
    lstring.c
    lstring.h
    lstrlib.c
    ltable.c
    ltable.h
    ltablib.c
    ltm.c
    ltm.h
    lua.h
    luaconf.h
    lualib.h
    lundump.c
    lundump.h
    lutf8lib.c
    lvm.c
    lvm.h
    lzio.c
    lzio.h
    )
