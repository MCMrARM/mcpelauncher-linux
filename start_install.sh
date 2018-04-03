#!/bin/bash
git submodule init && git submodule update
./setup_cef.sh
./download_icon.sh
./setup_bin_libs.sh
mkdir build/ && cd build/
cmake ..
make -j$(nproc)
