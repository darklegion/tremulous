# Windows, Linux and OSX

The GrangerHub Tremulous engine is supported on Windows, Linux and OSX. Builds
are automaticly produced by our CI/CD system.

# CI/CD 

The Tremulous engine is supported by a CI/CD (Continuous Integration/Continous
Delivery) using TravisCI and Github. Each time commits are pushed to the github
repository, TravisCI will verify that each of the MacOSX, Linux and Windows builds
are successful. Releases are generated automatically anytime a tag is pushed to
the repository; TravisCI will then package the build with the assets and push
the build artifacts to the Github releases page. Upon completion a notification
is sent to the GrangerHub Slack.

Hilights:
* TravisCI
* Automated builds
* Automated release generation

TODO: 
* Add Appveyor
* Reenable Coverity scans

# CMake support

A CMake system exists to build Tremulous on Linux and OSX. The CMake setup also
supports building QVM's using the in-tree QVM toolchain.

TODO: 
* Windows
* Appveyor Windows builds


# C++14

The engine has been modernized to compile at the C++14 standard. This was originally
done to simplify supporting Lua using the SOL2 Library, however this conversion has 
also resulted in a significantly improved stability due to better memory initialization.
It's worth noting that the renderer's have not been converted to C++14 as we currently
piggyback off ioquake3's renderer's (maintained by SmileTheory).

# Lua

The engine has an embedded Lua runtime and several API's are exposed from the engine,
including:

* Cvars
* Binds
* Passing commands to server
* Nettle (Crypto library)
* HTTP Client/Restful
* JSON (rapidjson)

# Multi-protocol

Support for Tremulous 1.1.0, Tremulous 1.2.0 (GPP) and the never released version from
DarkLegion's master branch are supported. A client can connect to any of those protocols. 
A server can simultaneously serve to all those protocols. QVM version detection on the client
which magically determines the correct interface required for a QVM.

This work is thanks to `/dev/humancontroller`.

# Auto-update 

Autoupdates are not fully functional, problems with the minizip implementation require additional
work to work.

Users can however check the latests updates from the main menu in the client, which queries the 
GrangerHub github for the latest release.

TODO: 
    * Lua library for ZIP files that preserves executable permissions 

# Restful HTTP Client

An HTTP Rest API is provided to both C++ and Lua interfaces.


# JSON Support

JSON support is provided via the lovely RapidJSON project. An interfaces is provided for both
C++ and Lua.

# Filesystem Stability

The filesystem has been refactored to be stable from startup. It is not possible for a server to
clobber the default QVM/UI.

FIXME:
    * Currently breaks mod loading support from the client main menu.


