#!/bin/bash
platform=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
if [[ "$platform" == "linux" ]]; then
    echo "yes" | sudo apt-add-repository ppa:zoogie/sdl2-snapshots
    sudo apt-get update -qq
    sudo apt-get remove -qq -y mingw32
    sudo apt-get install -q -y libgl1-mesa-dev libsdl2-dev libfreetype6-dev mingw-w64 g++-mingw-w64
fi
#elif [[ "$platform" == "darwin" ]]; then

