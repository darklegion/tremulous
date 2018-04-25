add_library (
    common STATIC
    cm_load.c
    cm_local.h
    cm_patch.c
    cm_patch.h
    cm_polylib.c
    cm_polylib.h
    cm_public.h
    cm_test.c
    cm_trace.c
    cmd.cpp
    common.c
    cvar.cpp
    dialog.h
    files.c
    huffman.c
    ioapi.c
    ioapi.h
    json.h
    md4.c
    md5.c
    msg.c
    net_chan.c
    net_ip.c
    parse.c
    puff.c
    puff.h
    q3_lauxlib.cpp
    q3_lauxlib.h
    q_math.c
    q_platform.h
    q_shared.c
    q_shared.h
    qcommon.h
    qfiles.h
    surfaceflags.h
    unzip.c
    unzip.h
    vm.c
    vm_interpreted.c
    vm_local.h
    vm_none.c
    vm_powerpc.c
    vm_powerpc_asm.c
    vm_powerpc_asm.h
    vm_sparc.c
    vm_sparc.h
    vm_x86.c
)

include_directories( ${RESTCLIENT_INCLUDES_DIR} )

set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14" )
