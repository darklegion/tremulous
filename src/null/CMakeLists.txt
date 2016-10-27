add_library (
    null STATIC
    mac_net.c
    null_client.c
    null_glimp.c
    null_input.c
    null_main.c
    null_net.c
    null_snddma.c
)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11" )
