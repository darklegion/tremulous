#!/bin/bash
P=$(uname | sed -e 's/_.*//' | tr '[:upper:]' '[:lower:]' | sed -e 's/\//_/g')
PLATFORM=${PLATFORM:-$P}
failed=0

if [[ $PLATFORM != "linux" ]]; then
    rm -rf build
    make V=1 release || failed=1
    ./misc/download-paks.sh
fi

if [[ $failed -eq 1 ]]; then
	echo "Build failure."
else
	echo "Build successful."
fi

exit $failed

