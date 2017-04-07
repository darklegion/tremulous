#!/bin/bash

URL="https://github.com/wtfbbqhax/tremulous-data/raw/master/"

packages="
data-1.1.0.pk3
data-gpp1.pk3
map-arachnid2-1.1.0.pk3
map-atcs-1.1.0.pk3
map-karith-1.1.0.pk3
map-nexus6-1.1.0.pk3
map-niveus-1.1.0.pk3
map-transit-1.1.0.pk3
map-tremor-1.1.0.pk3
map-uncreation-1.1.0.pk3
"

for dir in ./build/*; do
    if [[ ! -d $dir ]]; then
        continue;
    fi

    # Download

    if [[ $dir == "./build/release-darwin-x86_64" ]]; then
        pushd $dir/Tremulous.app/Contents/MacOS/gpp/ 
    else
        pushd $dir/gpp
    fi

    for i in $packages; do
        if [[ -e $package ]]; then
            rm -f $package # only want 1 copy
        fi
        curl -OL $URL/$i
    done

    popd

    # Repackage

    pushd $dir

    if [[ $dir == "./build/release-darwin-x86_64" ]]; then
        zip -r ../$(basename $dir).zip Tremulous.app
    else
        zip -r ../$(basename $dir).zip gpp/*.pk3
    fi

    popd
done

