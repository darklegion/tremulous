#!/bin/bash

# This file is used inside the docker image. Do not run standalone.

if [ ! -f /.dockerenv ];then # dockerenv is present on all docker containers
	(>&2 echo -e "\e[31mRefusing to run outside docker.\e[0m")
	exit 1;
fi

if [ -z "$(ls /root/.tremulous/gpp/)" ] && [ "$DOWNLOAD" != "0" ];then
	#case empty
	echo -e "\e[33mDownloading standard assets.\e[0m"
	if $(wget http://dl.grangerhub.com/files/downloads/clients/data-1.1.0.pk3 -qO- > /root/.tremulous/gpp/data-1.1.0.pk3 && \
	wget http://dl.grangerhub.com/files/downloads/clients/data-gpp1.pk3 -qO- > /root/.tremulous/gpp/data-gpp1.pk3); then
		echo -e "\e[32mDownloaded standard assets.\e[0m"
	else
		echo -e "\e[1;\e[41mStandard asset download failed, is the machine connected to the internet?\e[0m"
	fi
else
	echo -e "\e[1;\e[42mAsset directory not empty, will attempt to read from it.\e[0m"
fi

apt purge wget -y

if [ "$SERVER" != "0" ];then
	/usr/local/games/tremulous/tremded
else
	/usr/local/games/tremulous/tremulous
fi
