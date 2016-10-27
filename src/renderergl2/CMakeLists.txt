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

set(GLSL_SHADERS
    glsl/bokeh_fp.c
    glsl/bokeh_vp.c
    glsl/calclevels4x_fp.c
    glsl/calclevels4x_vp.c
    glsl/depthblur_fp.c
    glsl/depthblur_vp.c
    glsl/dlight_fp.c
    glsl/dlight_vp.c
    glsl/down4x_fp.c
    glsl/down4x_vp.c
    glsl/fogpass_fp.c
    glsl/fogpass_vp.c
    glsl/generic_fp.c
    glsl/generic_vp.c
    glsl/lightall_fp.c
    glsl/lightall_vp.c
    glsl/pshadow_fp.c
    glsl/pshadow_vp.c
    glsl/shadowfill_fp.c
    glsl/shadowfill_vp.c
    glsl/shadowmask_fp.c
    glsl/shadowmask_vp.c
    glsl/ssao_fp.c
    glsl/ssao_vp.c
    glsl/texturecolor_fp.c
    glsl/texturecolor_vp.c
    glsl/tonemap_fp.c
    glsl/tonemap_vp.c
    )

add_library(
    renderergl2 STATIC
    #
    tr_local.h
    tr_dsa.h
    tr_extramath.h
    tr_extratypes.h
    tr_fbo.h
    tr_postprocess.h
    #
    tr_animation.c
    tr_backend.c
    tr_bsp.c
    tr_cmds.c
    tr_curve.c
    tr_dsa.c
    tr_extensions.c
    tr_extramath.c
    tr_fbo.c
    tr_flares.c
    tr_glsl.c
    tr_image.c
    tr_image_dds.c
    tr_init.c
    tr_light.c
    tr_main.c
    tr_marks.c
    tr_mesh.c
    tr_model.c
    tr_model_iqm.c
    tr_postprocess.c
    tr_scene.c
    tr_shade.c
    tr_shade_calc.c
    tr_shader.c
    tr_shadows.c
    tr_sky.c
    tr_subs.c
    tr_surface.c
    tr_vbo.c
    tr_world.c
    )

target_link_libraries( renderergl2 renderercommon ${SDL2_LIBRARIES} )
