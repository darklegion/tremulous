FROM ubuntu:yakkety
WORKDIR /usr/src
COPY . /usr/src/
RUN apt update && apt install -y libgl1-mesa-dev libsdl2-dev libfreetype6-dev mingw-w64 g++-mingw-w64 g++-multilib git zip vim-nox
#RUN USE_INTERNAL_LUA=1 USE_RESTCLIENT=1 PLATFORM=mingw32 make -j 2
#RUN USE_INTERNAL_LUA=1 USE_RESTCLIENT=1 make -j 2

#RUN git clone https://github.com/wtfbbqhax/tremulous.git
#RUN cd tremulous 
#RUN git checkout lua
#RUN USE_INTERNAL_LUA=1 PLATFORM=mingw32 ARCH=x86_64 make -j 3
#RUN USE_INTERNAL_LUA=1 ARCH=x86_64 make -j 3

