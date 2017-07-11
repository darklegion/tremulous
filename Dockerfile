FROM ubuntu:yakkety
WORKDIR /usr/src
RUN apt update 
RUN apt install -y curl cmake libgl1-mesa-dev libsdl2-dev libcurl4-openssl-dev libopenal-dev libfreetype6-dev mingw-w64 g++-mingw-w64 g++-multilib git zip vim-nox
