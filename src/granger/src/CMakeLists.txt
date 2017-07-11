add_subdirectory(lua)
add_subdirectory(nettle)
add_subdirectory(premake)

add_executable(granger
        getopt.h
        lnettlelib.c
        lnettlelib.h
        main.c
        strvec.c
        strvec.h
        )

if(APPLE)
  add_definitions(-DLUA_USE_MACOSX)
endif(APPLE)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  add_definitions(-DLUA_USE_LINUX)
endif()

add_definitions (
    #-DLUA_COMPAT_5_2
    -DNDEBUG
    -mfpmath=sse
    -ffast-math
    -DGRANGER
    )

include_directories (
    include
    )
target_link_libraries(granger granger_lua granger_nettle premake)

if (NOT WIN32)
    target_link_libraries(granger m dl)
endif()

if(WIN32)
        add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)

if (APPLE)
        macro(ADD_FRAMEWORK fwname appname)
                find_library(FRAMEWORK_${fwname}
            NAMES ${fwname}
            PATHS ${CMAKE_OSX_SYSROOT}/System/Library
            PATH_SUFFIXES Frameworks
            NO_DEFAULT_PATH)
            if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
                        MESSAGE(ERROR ": Framework ${fwname} not found")
                else()
                        TARGET_LINK_LIBRARIES(${appname} "${FRAMEWORK_${fwname}}/${fwname}")
                        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
                endif()
        endmacro(ADD_FRAMEWORK)

        add_framework(CoreServices granger)
        add_framework(Security granger)
endif()
