add_library(
    script_rapidjson STATIC
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
    ${CMAKE_CURRENT_SOURCE_DIR}/../lua-5.3.3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../sol
    ${CMAKE_CURRENT_SOURCE_DIR}/../rapidjson
    ${CMAKE_CURRENT_SOURCE_DIR}/../qcommon
    )

target_link_libraries(
    script_rapidjson
    lua
    )

# command /usr/bin/clang++ -mdynamic-no-pic -Isrc/lua-5.3.3/include -DNDEBUG  -arch x86_64 -mfpmath=sse -ffast-math -o build/release-darwin-x86_64/script/rapidjson/document.o -c src/script/rapidjson/document.cpp

