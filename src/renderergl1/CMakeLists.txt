include("${CMAKE_SOURCE_DIR}/cmake/SDL2.cmake")

find_package(OpenGL)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../jpeg-8c
    ${CMAKE_CURRENT_SOURCE_DIR}/../renderercommon 
    ${SDL2_INCLUDE_DIRS} 
    )

set(renderergl1_SRCS
    tr_animation.cpp
    tr_backend.cpp
    tr_bsp.cpp
    tr_cmds.cpp
    tr_curve.cpp
    tr_flares.cpp
    tr_image.cpp
    tr_init.cpp
    tr_light.cpp
    tr_local.h
    tr_main.cpp
    tr_marks.cpp
    tr_mesh.cpp
    tr_model.cpp
    tr_model_iqm.cpp
    tr_scene.cpp
    tr_shade.cpp
    tr_shade_calc.cpp
    tr_shader.cpp
    tr_shadows.cpp
    tr_sky.cpp
    tr_subs.cpp
    tr_surface.cpp
    tr_world.cpp
    tr_local.h
    ${CMAKE_SOURCE_DIR}/src/common/puff.cpp
    ${CMAKE_SOURCE_DIR}/src/common/q_shared.c
    ${CMAKE_SOURCE_DIR}/src/common/q_math.c
    )

if(NOT USE_RENDERER_DLOPEN)
    add_library(
        renderergl1 STATIC
        ${renderergl1_SRCS}
        )

    target_link_libraries(
        renderergl1 renderercommon
        ${SDL2_LIBRARIES}
        )
else(NOT USE_RENDERER_DLOPEN)
    add_library(
        renderergl1 SHARED
        ${renderergl1_SRCS}
        )
    target_link_libraries(
        renderergl1
        renderercommon
        ${FRAMEWORKS}
        ${OPENGL_LIBRARIES}
        ${SDL2_LIBRARIES}
        )

endif(NOT USE_RENDERER_DLOPEN)


