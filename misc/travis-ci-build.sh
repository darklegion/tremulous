#!/bin/bash
P=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
PLATFORM=${PLATFORM:-$P}
failed=0

if [[ $PLATFORM == "darwin" ]]; then
    rm -rf build
    USE_FREETYPE=0 USE_RESTCLIENT=1 USE_INTERNAL_LUA=1 make -j 2 release || failed=1
    ./misc/download-paks.sh
fi

if [[ $failed -eq 1 ]]; then
	echo "Build failure."
else
	echo "Build successful."
fi

exit $failed

