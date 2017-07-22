#!/usr/bin/env bash

#Determines libs used
if grep -qi "amd" /proc/cpuinfo;  then
  /usr/bin/cp -r libs/AMD/* libs/
  printf "Using compatibility libs"
  sleep 3
fi

#Compiles mcpelauncher
cmake .
make

#Checks for complete build
if [ ! -e "mcpelauncher" ]; then
  echo "Error: mcpelauncher missing. Build failed"
  exit
fi

#Moves compiled files to new dir
sudo mkdir /usr/share/mcpelauncher
sudo cp -t /usr/share/mcpelauncher mcpelauncher extract.sh LICENSE mcpelauncher.desktop MCPEicon.png
sudo cp -r libs /usr/share/mcpelauncher/libs
cd /usr/share/mcpelauncher

#Acquires apk
printf "\nWhich method would you like to use to acquire an APK?\n"
printf "1) Google-Play-API (currently broken)\n"
printf "2) Hosted download (ONLY USE IF YOU OWN MCPE!)\n"
printf "3) Local file\n"
printf "\nEnter your selection: "
read answer
echo "$answer"
#Google-Play-API
if [[ "$answer" == "1" ]]; then
  git clone https://github.com/MCMrARM/Google-Play-API.git
  cd Google-Play-API
  cmake .
  make
  ./gplaydl -tos -a com.mojang.minecraftpe
  sudo cp *.apk /usr/share/mcpelauncher
  cd /usr/share/mcpelauncher
  sudo rm -R Google-Play-API
fi

#Hosted apk
if [[ "$answer" == "2" ]]; then
  wget https://kris27mc.github.io/files/mcpe.apk
#  /usr/bin/cp mcpe.apk ~/mcpelauncher
fi

#Local file
if [[ "$answer" == "3" ]]; then
  printf "Please enter the full path to your apk.\n"
  printf "Path to APK: "
  read -e pathtoapk
  if grep mcpe.apk <<< echo "$pathtoapk"; then
    sudo cp "$pathtoapk" /usr/share/mcpelauncher/mcpe-new.apk
  else
    sudo cp "$pathtoapk" /usr/share/mcpelauncher
  fi
fi

#Extracts apk into assets
if [[ "$answer" == "1" || "$input" == "3" ]]; then
  if [ -f "mcpe.apk" ]; then
    sudo mkdir oldapks
    sudo mv mcpe.apk oldapks
    sudo mv *.apk mcpe.apk
  fi
fi

sudo ./extract.sh mcpe.apk
sudo chmod -R 777 /usr/share/mcpelauncher

#Creates desktop launcher
cp mcpelauncher.desktop ~/.local/share/applications

printf "\nWould you like to create a shortcut on your desktop? (y/n)\n"
read input
if [[ $input == "Y" || "$input" == "y" ]]; then
  printf "Creating a shortcut..."
  cp mcpelauncher.desktop ~/Desktop
else
  printf "No desktop shortcut created."
fi
exit
