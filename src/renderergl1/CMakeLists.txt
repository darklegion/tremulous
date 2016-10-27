#FIXME: Link against prebuilt SDL2 
#FIND_LIBRARY(SDL2_LIBRARIES NAMES SDL2main HINTS $ENV{SDL2DIR} PATHS ${CMAKE_SOURCE_DIR}/src/libs/macosx)
if(APPLE)
 set(SDL2_PREFIX "/usr/local")
 set(SDL2_LIBRARIES "-L${SDL2_PREFIX} -lSDL2")
 set(SDL2_INCLUDE_DIRS "${SDL2_PREFIX}/include/SDL2")
else(APPLE)
 set(SDL2_PREFIX "/usr")
 set(SDL2_LIBRARIES "-L${SDL2_PREFIX} -lSDL2")
 set(SDL2_INCLUDE_DIRS "${SDL2_PREFIX}/include/SDL2")
endif(APPLE)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../jpeg-8c
    ${CMAKE_CURRENT_SOURCE_DIR}/../renderercommon 
    ${SDL2_INCLUDE_DIRS} 
    )

add_library (
    renderergl1 STATIC
    tr_animation.c
    tr_backend.c
    tr_bsp.c
    tr_cmds.c
    tr_curve.c
    tr_flares.c
    tr_image.c
    tr_init.c
    tr_light.c
    tr_local.h
    tr_main.c
    tr_marks.c
    tr_mesh.c
    tr_model.c
    tr_model_iqm.c
    tr_scene.c
    tr_shade.c
    tr_shade_calc.c
    tr_shader.c
    tr_shadows.c
    tr_sky.c
    tr_subs.c
    tr_surface.c
    tr_world.c
    tr_local.h
    )

target_link_libraries( renderergl1 renderercommon ${SDL2_LIBRARIES} )
