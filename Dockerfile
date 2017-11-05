FROM ubuntu:16.04
WORKDIR /mcpelauncher-linux
RUN dpkg --add-architecture i386; apt-get update; apt-get install -y zlib1g-dev:i386 libncurses5-dev:i386 libgles2-mesa-dev:i386 gcc-multilib g++-multilib libx11-dev:i386 linux-libc-dev:i386 uuid-dev:i386 libpng-dev:i386 libx11-dev:i386 libxext6:i386 pulseaudio:i386 libzip-dev:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386 protobuf-compiler libprotobuf-dev:i386 wget bzip2 git make
ADD https://cmake.org/files/v3.9/cmake-3.9.2-Linux-x86_64.tar.gz /tmp/cmake-3.9.2-Linux-x86_64.tar.gz
RUN tar -xz --strip=1 -C /usr/local -f /tmp/cmake-3.9.2-Linux-x86_64.tar.gz
RUN rm /tmp/cmake-3.9.2-Linux-x86_64.tar.gz
ADD ./ .
RUN ./setup_cef.sh
RUN ./setup_bin_libs.sh
RUN git submodule init && git submodule update
RUN mkdir build && cd build && cmake .. && make
