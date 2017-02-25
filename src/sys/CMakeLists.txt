add_library (
    sys STATIC
    con_log.cpp
    con_passive.cpp
    con_tty.cpp
    con_win32.cpp
    sys_loadlib.h
    sys_local.h
    sys_main.cpp
    sys_unix.cpp
    sys_win32.cpp
    win_resource.h
)

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
