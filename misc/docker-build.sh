#!/bin/bash
USE_RESTCLIENT=1 USE_INTERNAL_LUA=1 PLATFORM=mingw32 make -j 2
USE_RESTCLIENT=1 USE_INTERNAL_LUA=1 PLATFORM=linux make -j 2
chmod -R ugo+rw build
