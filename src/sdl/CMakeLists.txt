add_library (
    sdl STATIC
    sdl_gamma.c
    sdl_glimp.c
    sdl_icon.h
    sdl_input.c
    sdl_snd.c
)

set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11" )
