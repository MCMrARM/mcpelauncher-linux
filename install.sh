#!/usr/bin/env bash

#Determines libs used
if grep -qi "amd" /proc/cpuinfo;  then
  /usr/bin/cp -r libs/AMD/* libs/
fi

#Compiles
cmake .
make

#Checks for complete build
if ! ls | grep -q "mcpelauncher"; then
  echo "Error: mcpelauncher missing. Build failed"
  exit
fi

#Moves compiled files to new dir
mkdir ~/mcpelauncher
cp -t ~/mcpelauncher mcpelauncher extract.sh LICENSE mcpe.desktop MCPEicon.png
cp -r libs ~/mcpelauncher/libs
cd ~/mcpelauncher

#Acquires apk
#git clone https://github.com/MCMrARM/Google-Play-API.git
#cd Google-Play-API
#cmake .
#make
#./gplaydl -tos -a com.mojang.minecraftpe
wget https://kris27mc.github.io/files/mcpe.apk
cp *.apk ~/mcpelauncher
cd ~/mcpelauncher
#rm -R Google-Play-API

#Extracts apk into assets
if [-f "mcpe.apk"]; then
  mkdir oldapks
  mv mcpe.apk oldapks
fi
mv *.apk mcpe.apk
./extract.sh mcpe.apk

#Creates desktop launcher
 cp mcpe.desktop ~/.local/share/applications
