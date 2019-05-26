![Wtfbbqhax/Tremulous/tremulous-banner.jpg](misc/tremulous-banner.jpg)

[![Travis branch](https://travis-ci.org/GrangerHub/tremulous.svg?branch=master)](https://travis-ci.org/GrangerHub/tremulous)
[![Coverity Scan](https://img.shields.io/coverity/scan/9866.svg?maxAge=3600)](https://scan.coverity.com/projects/wtfbbqhax-tremulous)

# How to Install from GrangerHub's Release Page

You can install the latest released binaries from GrangerHub's release page,
following these steps:

* Download the most recent .zip file for your platform from https://github.com/GrangerHub/tremulous/releases
 - The 64 bit Windows release would be named release-mingw32-x86_64.zip
 - The 64 bit Linux release would be named release-linux-x86_64.zip
 - The 64 bit Mac OS X release would be named release-darwin-x86_64.zip
* Unzip the release .zip anywhere
* Run the tremulous.exe from the unzipped release directory.
 - When you run the tremulous.exe binary for the first time, it may go through a bootstrap (download needed assets, generate an RSA key) process which may take a few minutes.
* You can simply continue to run the new client from wherever the unzipped release folder is, or you could optionally replace your old Tremulous binaries with the binaries in the unzipped release directory, but make sure that you copy all of the contents of the unzipped release directory to where ever you move the released binaries to.  **_Backup your existing Tremulous if you intend on replacing the binaries._**

# Building from Source

## Dependencies

If you want to build against system libraries, the following packages are necessary:

### Linux:

* GCC/G++ (version 6+ is recommend)
* CMake (Optional)
* CURL (for downloading assets)
* rsync (for asset management)
* Zip (the tool not the library)
* Libgl1-mesa
* LibSDL2
* LibCURL4
* LibOpenAL
* Libfreetype6
* Lua 5.2

On Ubuntu Yakkety (specifically) you can install all the packages necessary with the following:

```
apt install -y cmake libgl1-mesa-dev libsdl2-dev libcurl4-openssl-dev libopenal-dev libfreetype6-dev mingw-w64 g++-mingw-w64 g++-multilib git zip vim-nox curl rsync

```

### Mingw32 (win32 cross compile)

Mingw32 requires `USE_INTERNAL_LIBS=1`

* Mingw-w64
* g++-mingw-w64

Currently there is no native Windows build setup, all builds are cross compiled on Linux.

### OSX

TBD

## How to Install from the Source Code

For Linux and Mac OS X builds, follow these steps.

```bash
git clone https://github.com/GrangerHub/tremulous.git
cd tremulous
make
# cd build/release-darwin-x86_64/
# cd build/release-linux-x86_64/
./tremulous
```

# How to build for Win64

Windows binaries can be built on non-Unix environments using a Docker container.
Click [here](https://www.docker.com/) to learn more about Docker.

Note that on Windows, you have to clone this Git repository with Unix line endings 
because the build system is still in Unix environment:

```bash
git clone -c core.eol=lf ...
```
(WARNING: Be careful to not commit back library files from this clone since their line-endings may change)

To build the Tremulous binaries do:

```bash
docker run -t -i -e PLATFORM=mingw32 -v $(pwd):/usr/src grangerhub/tremulous13:latest ./misc/docker-build.sh
```

On Windows, make sure in the Docker Desktop settings that the drive you have the source 
code is selected under "Shared Drives". Then replace `$(pwd)` with the actual folder where the 
source resides (e.g. `"C:\Users\...\tremulous"`).

To run:

```bash
cd build/release-mingw32-x86_64/
./tremulous.exe
```

(use backslashes `\` if you are using Command Prompt).

## Troubleshooting issues with running on Windows 

- If you cannot select the high-resolution display modes inside the game, check Windows 
Compatibility Settings under the Properties menu when you right-click the 
Tremulous executable. Select "Change high DPI  settings" and check the
"Override high DPI scaling behavior. Scaling performed by: Application".
- If your FPS is low, make sure you are running the game when plugged into 
power (as opposed to laptop battery).

# Where do I get the assets?

The Tremulous client binary by default will attempt to retrieve the assets automatically (Known as the Pk3 bootstrap).
However, sometimes this does not work as expected.

If you need to download the assets yourself; they are available here:

https://github.com/wtfbbqhax/tremulous-data

Or you can checkout the entire repository:

```bash
git clone https://github.com/wtfbbqhax/tremulous-data.git
```

Copy the contents of the assets folder as a new `base/` folder in the release location.

# About Lua

This branch integrates Lua into the Tremulous runtime. This is a base branch for additional Lua api development to branch from.

# About Transform

This code allows customizing human player models with arbitray quake3 thirdparty md3-%.pk3's.

This adds the admin command `/transform [name|slot#] [modelname] <skin>` which is an admin abusive command to
force player model chanes.

This also includes voice menus branch


# Console Cvars

* New Cvar `scr_useShader` enable/disable use of the console shader vs color controls below. 
 - (0, 1) default 1 (enabled)
* New Cvar `scr_height` control console height.
 - 100: (e.g., 100%) Default
* New Cvar `scr_colorRed` console background red amount.
 - (0.0f - 1.0f) default 0.0f
* New Cvar `scr_colorGreen` console background green amount.
 - (0.0f - 1.0f) default 0.0f
* New Cvar `scr_colorBlue` console background blue amount.
 - (0.0f - 1.0f) default 0.0f
* New Cvar `scr_colorAlpha` console transparency.
 - (0.0f - 1.0f) default 0.8f
