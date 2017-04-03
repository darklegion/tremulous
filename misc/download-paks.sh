#!/bin/bash

packages="
https://github.com/wtfbbqhax/tremulous-data/raw/master/data-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/data-gpp1.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-arachnid2-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-atcs-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-karith-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-nexus6-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-niveus-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-travis-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-tremor-1.1.0.pk3
https://github.com/wtfbbqhax/tremulous-data/raw/master/map-uncreation-1.1.0.pk3
"

for dir in ./build/*; do
    if [[ ! -d $dir ]]; then
        continue;
    fi

    pushd $dir/gpp

    for i in $packages; do
        wget $i
    done

    popd
done
