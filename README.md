MCPE Linux Launcher
===================

## Required packages

```
    sudo apt-get install cmake zlib1g-dev:i386 libncurses5-dev:i386 libgles2-mesa-dev gcc-4.9 g++-4.9 gcc-4.9-multilib g++-4.9-multilib zlib1g-dev:i386 libx11-dev:i386 linux-libc-dev:i386 uuid-dev:i386 libpng-dev:i386 libx11-dev:i386 
```

If g++-4.9 fails to install and you're using Ubuntu 14.04 you may need to add the repository ppa:ubuntu-toolchain-r/test.

You'll also need to install 32-bit version of the graphic drivers (nvidia drivers ask you about that at installation, so
you may need to reinstall/reconfigure them; if you use mesa you'll need to install the libgles2-mesa-dev:i386 and
libegl1-mesa-dev:i386 packages)

## Compiling
This app uses cmake so it is enough to do:

```
    cmake .
    make
```

## Running
1. Clone this repository
2. Compile the launcher
3. You'll need to obtain a x86 MCPE .apk. The easiest way to do so is to use the
[Google Play downloader tool](https://github.com/MCMrARM/google_play_downloader) (it is an console app; remember that
you need to type 'y' when you are asked if you want to use x86 as the architecture, and you must have purchased MCPE
on the Google account you are logging in with; the package name of MCPE is `com.mojang.minecraftpe`)
4. After you have downloaded MCPE, place it in the directory where you'll be running this app, and run ./extract.sh _filename_
5. Run the launcher!

This launcher only works with the 0.13 beta version as of now.

If the extract script fails with an error about the .apk not being x86, it means that you have provided it a bad .apk.
You'll need to purchase MCPE on Google Play and use the downloader tool.

## License and thanks
Most of the code in this repo is licensed under BSD. This project uses libc, libstdc++, libz and libm - libraries
extracted from the Android OS. A modified version of libhybris is also included, which is licensed under GPL. This project
also uses the EGLUT library and FMOD library (for sound).
