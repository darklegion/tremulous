FROM debian:stable-slim as build
WORKDIR /usr/src
ENV DEBIAN_FRONTEND=noninteractive
RUN apt update -y
RUN apt install -y make gcc g++ libgl1-mesa-dev libsdl2-dev libcurl4-openssl-dev libopenal-dev libfreetype6-dev zip rsync
COPY . /usr/src/
RUN make -j

FROM debian:stable-slim as run
RUN apt update -y
RUN apt install -y libsdl2-2.0-0 libcurl4 libgl1 wget
RUN apt autoremove -y;apt clean;rm -rf /var/lib/{apt,dpkg,cache,log}/
COPY --from=build /usr/src/build/release-linux-x86_64/ /usr/local/games/tremulous/
COPY ./docker.sh /docker.sh
RUN chmod u+x /docker.sh
RUN mkdir -p /root/.tremulous/gpp

VOLUME /root/.tremulous/gpp/
EXPOSE 30720/udp
EXPOSE 30721/udp
EXPOSE 30722/udp

# Environment variable options: (passed via docker run -e)
# SERVER: if set to 1, will run a server; otherwise will run a client.
# DOWNLOAD: the container will download default maps and assets to the volume (if it is empty). set DOWNLOAD=0 to bypass this action.
ENV SERVER 0
ENV DOWNLOAD 1

ENTRYPOINT /bin/bash /docker.sh

# run client with:
#docker build -t tremulous:latest . && xhost +local:root && docker run --net=host -it -e DISPLAY=:0 -v /tmp/.X11-unix:/tmp/.X11-unix  --device /dev/snd tremulous:latest
# see http://wiki.ros.org/docker/Tutorials/GUI for more info on running GUI applications with docker.
# as running the game inside docker is heavily dependant on environment and requires questionable security practices, it is advised that one copies the built game out of the container through `docker cp`.

# run server with:
#docker build -t tremulous:latest . && docker run -it -e SERVER=1 -p 30720:30720 -v /home/<user>/tremulous:/root/.tremulous/gpp/ tremulous:latest