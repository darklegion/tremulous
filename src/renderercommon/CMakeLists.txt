include(${CMAKE_SOURCE_DIR}/cmake/SDL2.cmake)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c ${SDL2_INCLUDE_DIRS} )

add_library(
    renderercommon STATIC
    iqm.h
    qgl.h
    tr_common.h
    tr_font.cpp
    tr_image_bmp.cpp
    tr_image_jpg.cpp
    tr_image_pcx.cpp
    tr_image_png.cpp
    tr_image_tga.cpp
    tr_noise.cpp
    tr_public.h
    tr_types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdl/sdl_gamma.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../sdl/sdl_glimp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jaricom.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jccoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jccolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcdctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jchuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcinit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcomapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcparam.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcprepct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jcsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jctrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdapimin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdapistd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdarith.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdatadst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdatasrc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdcoefct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdcolor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jddctmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdhuff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdinput.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdmainct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdmarker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdmaster.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdmerge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdpostct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdsample.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jdtrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jerror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jfdctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jfdctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jfdctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jidctflt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jidctfst.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jidctint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jmemmgr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jmemnobs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jquant1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jquant2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/jpeg-8c/jutils.c
    )

add_definitions(
    -DFLOATING_POINT
    -DHAVE_LRINTF
    -DUSE_INTERNAL_JPEG
    -DUSE_RENDERER_DLOPEN
    ${SDL2_DEFINES}
    )

target_link_libraries( renderercommon ${SDL2_LIBRARIES} )
