include("${CMAKE_SOURCE_DIR}/cmake/SDL2.cmake")

find_package(OpenGL)

set(PARENT_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(
    ${PARENT_DIR}/jpeg-8c
    ${PARENT_DIR}/renderercommon 
    ${PARENT_DIR}/nettle-3.3
    ${SDL2_INCLUDE_DIRS} 
    )

include_directories(lib/embed-resource)
add_subdirectory(lib/embed-resource)

embed_resources(
    GLSL_SHADERS
    glsl/bokeh_fp.glsl
    glsl/bokeh_vp.glsl
    glsl/calclevels4x_fp.glsl
    glsl/calclevels4x_vp.glsl
    glsl/depthblur_fp.glsl
    glsl/depthblur_vp.glsl
    glsl/dlight_fp.glsl
    glsl/dlight_vp.glsl
    glsl/down4x_fp.glsl
    glsl/down4x_vp.glsl
    glsl/fogpass_fp.glsl
    glsl/fogpass_vp.glsl
    glsl/generic_fp.glsl
    glsl/generic_vp.glsl
    glsl/lightall_fp.glsl
    glsl/lightall_vp.glsl
    glsl/pshadow_fp.glsl
    glsl/pshadow_vp.glsl
    glsl/shadowfill_fp.glsl
    glsl/shadowfill_vp.glsl
    glsl/shadowmask_fp.glsl
    glsl/shadowmask_vp.glsl
    glsl/ssao_fp.glsl
    glsl/ssao_vp.glsl
    glsl/texturecolor_fp.glsl
    glsl/texturecolor_vp.glsl
    glsl/tonemap_fp.glsl
    glsl/tonemap_vp.glsl
    )

# XXX: GLSL_SOURCES files must be generated
# define DO_REF_STR
#   $(echo_cmd) "REF_STR $<"
#   rm -f $@
#   echo "const char *fallbackShader_$(notdir $(basename $<)) =" >> $@
#   cat $< | sed 's/^/\"/;s/$$/\\n\"/' >> $@
#   echo ";" >> $@
# endef
# 
# $(B)/renderergl2/glsl/%.c: $(RGL2DIR)/glsl/%.glsl
# 	$(DO_REF_STR)

set(RENDERERGL2_SRCS
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
    #
    #${CMAKE_SOURCE_DIR}/src/qcommon/cm_load.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cm_patch.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cm_polylib.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cm_test.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cm_trace.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cmd.cpp
    #${CMAKE_SOURCE_DIR}/src/qcommon/common.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/cvar.cpp
    #${CMAKE_SOURCE_DIR}/src/qcommon/files.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/huffman.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/ioapi.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/md4.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/md5.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/msg.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/net_chan.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/net_ip.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/parse.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/puff.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/q_shared.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/q3_lauxlib.cpp
    #${CMAKE_SOURCE_DIR}/src/qcommon/q_math.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/unzip.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/vm.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/vm_interpreted.c
    #${CMAKE_SOURCE_DIR}/src/qcommon/vm_x86.c
    #
    ${GLSL_SHADERS}
    )

if(NOT USE_RENDERER_DLOPEN)
    add_library(
        renderergl2 STATIC
        ${RENDERERGL2_SRCS}
        )

    target_link_libraries(
        renderergl2 renderercommon
        ${SDL2_LIBRARIES}
        )
else(NOT USE_RENDERER_DLOPEN)
    add_library(
        renderergl2 SHARED
        ${RENDERERGL2_SRCS}
        )

    set_property(
        TARGET renderergl2
        PROPERTY OUTPUT_NAME "renderer_opengl2"
        )

    set_property(
        TARGET renderergl2
        PROPERTY RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )

endif(NOT USE_RENDERER_DLOPEN)

target_link_libraries(
    renderergl2 
    renderercommon
    ${FRAMEWORKS}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    )
