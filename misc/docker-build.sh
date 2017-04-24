#!/bin/bash
failed=0

USE_RESTCLIENT=1 USE_INTERNAL_LUA=1 make -j 2 || failed=1

if [[ $failed -eq 1 ]]; then
    echo "Build failure."
    exit $failed
fi

./misc/download-paks.sh
chmod -R ugo+rw build
