MCPE Linux Launcher
===================

## Required packages

```
    sudo apt-get install -y cmake zlib1g-dev:i386 libncurses5-dev:i386 libgles2-mesa-dev:i386 gcc-multilib g++-multilib libx11-dev:i386 linux-libc-dev:i386 uuid-dev:i386 libpng-dev:i386 libx11-dev:i386 libxext6:i386 pulseaudio:i386 libzip-dev:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386 protobuf-compiler libprotobuf-dev:i386 wget bzip2
```

If g++-4.9 fails to install and you're using Ubuntu 14.04 you may need to add the repository ppa:ubuntu-toolchain-r/test.

You'll also need to install 32-bit version of the graphic drivers (nvidia drivers ask you about that at installation, so
you may need to reinstall/reconfigure them; if you use mesa you'll need to install the libgles2-mesa-dev:i386 and
libegl1-mesa-dev:i386 packages)

You may also need to do `sudo dpkg --add-architecture i386` if you have never installed i386 packages before.

## Compiling
This app uses cmake so it is enough to do:

```
    ./setup_cef.sh
```

## Running
1. Clone this repository
2. Compile the launcher
3. You'll need to obtain a x86 MCPE .apk. The easiest way to do so is to use the
[Google Play downloader tool](https://github.com/MCMrARM/Google-Play-API) (it is an console app; remember that
you need to type 'y' when you are asked if you want to use x86 as the architecture, and you must have purchased MCPE
on the Google account you are logging in with; the package name of MCPE is `com.mojang.minecraftpe`)
4. After you have downloaded MCPE, place it in the directory where you'll be running this app, and run ./extract.sh _filename_
5. Run the launcher!

If the extract script fails with an error about the .apk not being x86, it means that you have provided it a bad .apk.
You'll need to purchase MCPE on Google Play and use the downloader tool.

## License and thanks
Most of the code in this repo is licensed under BSD. This project uses libc, libstdc++, libz and libm - libraries
extracted from the Android OS. A modified version of libhybris is also included, which is licensed under GPL. This project
also uses the EGLUT library and FMOD library (for sound).
