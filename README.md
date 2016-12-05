![Wtfbbqhax/Tremulous/tremulous-banner.jpg](https://github.com/wtfbbqhax/tremulous-art/blob/master/branding/tremulous-banner.jpg)

[![Travis branch](https://travis-ci.org/wtfbbqhax/tremulous.svg?branch=lua)](https://travis-ci.org/wtfbbqhax/tremulous)
[![Coverity Scan](https://img.shields.io/coverity/scan/9866.svg?maxAge=3600)](https://scan.coverity.com/projects/wtfbbqhax-tremulous)

# How to Install

For Linux and Mac OS X builds, follow these steps.

```bash
git clone https://github.com/wtfbbqhax/tremulous.git
cd tremulous
make
# cd build/release-darwin-x86_64/
# cd build/release-linux-x86_64/
./tremulous
```

# How to build for Win64

Windows binaries are built using a Docker container.
Click [here](https://www.docker.com/) to learn more about Docker.

```bash
docker build -t wtfbbqhax/tremulous:v2 .
docker run -t -i -v $(pwd):/usr/src wtfbbqhax/tremulous:v2 make USE_RESTCLIENT=1 USE_INTERNAL_LUA=1 PLATFORM=mingw32
```

# Where do I get the assets?

The Tremulous client binary by default will attempt to retrieve the assets automatically (Known as the Pk3 bootstrap).
However, sometimes this does not work as expected.

If you need to download the assets yourself; they are available here:

https://github.com/wtfbbqhax/tremulous-data

Or you can checkout the entire repository:

```bash
git clone https://github.com/wtfbbqhax/tremulous-data.git
```

# About Lua

This branch integrates Lua into the Tremulous runtime. This is a base branch for additional Lua api development to branch from.

# About Transform

This code allows customizing human player models with arbitray quake3 thirdparty md3-%.pk3's.

This adds the admin command `/transform [name|slot#] [modelname] <skin>` which is an admin abusive command to
force player model chanes.

This also includes voice menus branch
