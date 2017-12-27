MCPE Linux Launcher
===================

## Required packages on Ubuntu

- Minimal: `cmake gcc-multilib g++-multilib zlib1g-dev:i386 libx11-dev:i386 libzip-dev:i386 libpng-dev:i386 uuid-dev:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 libgles2-mesa-dev:i386`
- CEF (supports Xbox Live): `libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386`
- First Time Setup (allows you to log in to Google Play and download the .apk): `protobuf-compiler libprotobuf-dev:i386` (requires the CEF dependencies as well)

You'll also need to install 32-bit version of the graphic drivers (nvidia drivers ask you about that at installation, so
you may need to reinstall/reconfigure them; if you use mesa you'll need to install the libgles2-mesa-dev:i386 and
libegl1-mesa-dev:i386 packages)

You may also need to do `sudo dpkg --add-architecture i386` if you have never installed i386 packages before.

## Compiling

If you want to build with CEF first do: `./setup_cef.sh`
```
    ./download_icon.sh
    ./setup_bin_libs.sh
    cmake .
    make
```

## Running
1. Clone this repository
2. Compile the launcher
3. You'll need to obtain a x86 MCPE .apk. If you have built the application with the First Time Setup, you'll be able to log in to your Google account the first time you start the launcher and download it. If you can not do this then after you have downloaded MCPE, run: mcpelauncher extract _filename_
5. Run the launcher!

If the extract script fails with an error about the .apk not being x86, it means that you have provided it a bad .apk.
You'll need to purchase MCPE on Google Play and use the first time setup.

## License and thanks
Most of the code in this repo is licensed under BSD. This project uses libc, libstdc++, libz and libm - libraries
extracted from the Android OS. A modified version of libhybris is also included, which is licensed under GPL. This project
also uses the EGLUT library and FMOD library (for sound).
