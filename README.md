```
 _____                         _                    ____ ____  ____  
|_   _| __ ___ _ __ ___  _   _| | ___  _   _ ___   / ___|  _ \|  _ \ 
  | || '__/ _ \ '_ ` _ \| | | | |/ _ \| | | / __| | |  _| |_) | |_) |
  | || | |  __/ | | | | | |_| | | (_) | |_| \__ \ | |_| |  __/|  __/ 
  |_||_|  \___|_| |_| |_|\__,_|_|\___/ \__,_|___/  \____|_|   |_|    
                                                                     

```
[![Build Status](https://travis-ci.org/wtfbbqhax/tremulous.svg?branch=blowfish-alpha-1)](https://travis-ci.org/wtfbbqhax/tremulous)
[![Coverity Scan](https://img.shields.io/coverity/scan/9866.svg)](https://scan.coverity.com/projects/wtfbbqhax-tremulous)

---
### Workspace for a Modern Tremulous Distribution

Testing builds are comming soon. 

Development is done in 2 week sprint cycles; Test builds will be distributed after every cycle.

## Tasks by priority

#### Feature list

1. Auto-update + Launcher
2. 1.1-ish Gameplay QVM
3. Integrate the new Tremded
4. Voice Commands UI
6. Merge "more-give" et, al. new devmap luxuries.
7. Player Model Customization

#### Forthcoming

1. Native integratation of Launcher and client (requires C++11)
2. Integrate new Tremded source code (requires C++11)
3. Integrate new Admin system changes (requires C++11)
5. Pull in All my gimmicky mod's from devmap-wonderland and the non-public grangerhub repo

#### Completed

1. Merge upstream ioq3 fixes for OSX
 - Required for C++11 build support
2. C++11 compilation and linking support
 - Required by Tremded rewrite
 - Required by Launcher integration

---

#### Backlog

1. Playmap (server side)
2. SQLite3 (server side)
3. Protocol 69 (maybe)
4. Lua backend plugins (logging, networking, databases, ie messaging systems)
5. Lua gameplay plugins (weapons, buildables, etc.)
6. Protocol "tremulous" (i.e., darklegion/master)
