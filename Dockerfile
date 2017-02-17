FROM ubuntu:yakkety
WORKDIR /usr/src
COPY . /usr/src/
RUN apt update 
RUN apt install -y cmake libgl1-mesa-dev libsdl2-dev libcurl4-openssl-dev libopenal-dev libfreetype6-dev mingw-w64 g++-mingw-w64 g++-multilib git zip vim-nox

RUN USE_INTERNAL_LUA=1 USE_RESTCLIENT=1 PLATFORM=mingw32 make -j 4
RUN USE_INTERNAL_LUA=1 USE_RESTCLIENT=1 make -j 4
RUN mkdir cbuild && cd cbuild && cmake .. && make -j 4 all game.qvm cgame.qvm ui.qvm

#RUN git clone https://github.com/GrangerHub/tremulous.git
#RUN cd tremulous 
#RUN git checkout lua
#RUN USE_INTERNAL_LUA=1 PLATFORM=mingw32 ARCH=x86_64 make -j 3
#RUN USE_INTERNAL_LUA=1 ARCH=x86_64 make -j 3
