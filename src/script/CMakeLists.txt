add_library(
    script_api STATIC
    client.h
    cvar.h
    http_client.h
    lnettlelib.c
    lnettlelib.h
    lnettlelib.h
    nettle.h
    rapidjson.h
    rapidjson/document.cpp
    rapidjson/file.hpp
    rapidjson/luax.hpp
    rapidjson/rapidjson.cpp
    rapidjson/schema.cpp
    rapidjson/userdata.hpp
    rapidjson/values.cpp
    rapidjson/values.hpp
    )


include_directories(
    # these are in global namespace
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/lua-5.3.3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/sol
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    # these have they're own namespace
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/nettle-3.3
    ${CMAKE_CURRENT_SOURCE_DIR}/../../external/rapidjson
    )

target_link_libraries(
    script_api
    lua
    )
