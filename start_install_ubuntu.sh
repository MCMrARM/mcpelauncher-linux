#!/bin/bash
sudo dpkg --add-architecture i386
sudo apt install libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386 libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386 protobuf-compiler libprotobuf-dev:i386
git submodule init && git submodule update
./setup_cef.sh
./download_icon.sh
./setup_bin_libs.sh
mkdir build/ && cd build/
cmake ..
make -j$(nproc)
