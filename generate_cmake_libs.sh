#!/usr/bin/env bash
set -x
#Generate compile_commands.json
#gmake clean
#gmake -j 
#gmake compile_commands.json


# Extract renderer_common source files
targets="tremulous 
        tremded 
        renderergl1 
        renderergl2 
        renderercommon 
        qcommon 
        lua 
        nettle"


#DB=compile_commands.json

#function select_()
#{
#    jq "'[ .[] as \$Entry | if (\$Entry.target == \"$1\") then \$Entry else empty end ]'" < $DB > $1-tmp.json
##
##    jq "'{ \"library\": [.[].relative_file], \"command\": .[0].command }'" < $1-tmp.json > $1.json
##
##    rm -f $1-tmp.json
#}

#for i in $targets
#do
#    select_ $i
#done
#set +x
jq -c '[ .[] as $Entry | if ($Entry.target == "tremulous") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_executable": { "tremulous": [.[].relative_file] }, "target_link_libraries": { "tremulous": [ "lua", "qcommon", "nettle", "script" ] }, "include_directories": [ "src/lua-5.3.3/include", "src/script", "src/nettle-3.3", "src/zlib", "src/jpeg-8c", "src/restclient", "src/opusfile-0.5/include", "src/opus-1.1/silk/float", "src/opus-1.1/silk", "src/opus-1.1/celt", "src/opus-1.1/include", "src/libogg-1.3.1/include"], "command": .[0].command } ]'

jq -c '[ .[] as $Entry | if ($Entry.target == "script") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_library": { "script": [.[].relative_file] }, "command": .[0].command } ]'

jq -c '[ .[] as $Entry | if ($Entry.target == "lua") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_executable": { "lua": [.[].relative_file] }, "include_directories": [ "src/lua-5.3.3/include" ], "command": .[0].command } ]'

#jq -c '[ .[] as $Entry | if ($Entry.target == "tremulous") then $Entry else empty end]' < compile_commands.json |\
#    jq '[ { "add_executable": { "tremulous": [.[].relative_file] }, "add_target_libraries": [ "lua", "qcommon", "nettle", "script" ], "command": .[0].command } ]'
#
#jq -c '[ .[] as $Entry | if ($Entry.target == "tremulous") then $Entry else empty end]' < compile_commands.json |\ 
#     jq -c '[ { "add_executable": { .[0].target: [.[].relative_file] }, "command": .[0].command } ]' > tremulous.json 


# LUA
jq -c '[ .[] as $Entry | if ($Entry.target == "lua") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_library": [ "lua", "STATIC", .[].relative_file] }, "add_definitions": [ "-DNDEBUG", "-mfpmath=sse", "-ffast-math" ], "include_directories": [ "src/lua-5.3.3/include" ], "command": .[0].command } ]'

# Nettle
jq -c '[ .[] as $Entry | if ($Entry.target == "nettle") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_library": [ "nettle", "STATIC", .[].relative_file] }, "add_definitions": [ "-DNDEBUG", "-mfpmath=sse", "-ffast-math" ], "include_directories": [ "src/nettle-5.3.3/include" ], "command": .[0].command } ]'

jq -c '[ .[] as $Entry | if ($Entry.target == "qcommon") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_library": [ "qcommon", "STATIC", .[].relative_file] }, "command": .[0].command } ]'

##jq '{ "library": [.[].relative_file], "command": .[0].command }' < renderer_common.json

jq -c '[ .[] as $Entry | if ($Entry.target == "game") then $Entry else empty end]' < compile_commands.json |\
    jq '[ { "add_library": { "game": [.[].relative_file] }, "command": .[0].command } ]' >> src/game/CMakeLists.txt
