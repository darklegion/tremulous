FROM ubuntu:18.04
WORKDIR /usr/src
RUN apt update 
RUN apt install -y libsdl2-dev libgl1-mesa-dev libcurl4-openssl-dev libopenal-dev libfreetype6-dev 
RUN apt install -y g++ curl cmake git zip mingw-w64 g++-mingw-w64 g++-multilib rsync
