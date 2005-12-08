#! /bin/sh

gcc -E infopanes.def.h | grep "^[^#].*" > infopanes.def
