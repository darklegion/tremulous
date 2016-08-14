#!/bin/bash
platform=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
if [[ "$platform" == "linux" ]]; then
    echo ">>>>>>>>>>>>>>> Linux <<<<<<<<<<<<<<<"
    sudo apt-get update -qq
    sudo apt-get install -q -y libgl1-mesa-dev libsdl2-dev libfreetype6-dev mingw-w64 g++-mingw-w64
else
    echo ">>>>>>>>>>>>> Apple OSX <<<<<<<<<<<<<"
    brew update
    brew install freetype
fi

exit 0
