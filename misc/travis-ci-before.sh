#!/bin/bash
platform=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
if [[ "$platform" == "linux" ]]; then
    echo yes | sudo apt-add-repository ppa:zoogie/sdl2-snapshots
    sudo apt-get update -qq &> /dev/null
    sudo apt-get install -q -y libgl1-mesa-dev libsdl2-dev libfreetype6-dev mingw-w64 g++-mingw-w64 g++-multilib &> /dev/null
else
    brew update &> /dev/null
    brew install freetype --universal &> /dev/null

fi

exit 0
