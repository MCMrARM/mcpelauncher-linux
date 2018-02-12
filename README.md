MCPE Linux Launcher
===================

## Ubuntu

- Minimal: `cmake gcc-multilib g++-multilib zlib1g-dev:i386 libx11-dev:i386 libzip-dev:i386 libpng-dev:i386 libcurl4-openssl-dev:i386 libssl-dev:i386 libgles2-mesa-dev:i386 libudev-dev:i386 libevdev-dev:i386`
- CEF (supports Xbox Live): `libgtk2.0-0:i386 libgtkglext1:i386 libasound2:i386 libnss3:i386 libxss1:i386 libgconf2-4:i386 libxtst6:i386 libudev1:i386`
- First Time Setup (allows you to log in to Google Play and download the .apk): `protobuf-compiler libprotobuf-dev:i386` (requires the CEF dependencies as well)

You'll also need to install 32-bit version of the graphic drivers (nvidia drivers ask you about that at installation, so
you may need to reinstall/reconfigure them; if you use mesa you'll need to install the libgles2-mesa-dev:i386 and
libegl1-mesa-dev:i386 packages)

You may also need to do `sudo dpkg --add-architecture i386` if you have never installed i386 packages before.

To compile, the commands are as follows (if you want to build with CEF first run `./setup_cef.sh`):

```console
$ ./download_icon.sh
$ ./setup_bin_libs.sh
$ mkdir build/ && cd build/
$ cmake ..
$ make -j$(nproc) # This will use all cores on your system
```

## macOS

Make sure you've got XCode set up: `xcode-select --install` (when the dialog pops up, just say you want the tools). Next, make sure you have [Brew](https://brew.sh/) installed

The packages you'll need are as follows:

- cmake
- git (this will come pre-installed with xcode-select, but make sure it's up-to-date)

Everything else will be built automatically and statically linked against your executable. So **do not** install `libzip`, `ossp-uuid`, `glfw` or any other packages you think you need.

Next, you'll need to build the project. Run the provided scripts `./setup_bin_libs.sh` (to grab FMOD) and `download_icon.sh` (to grab the icon, which is currently not used)

The commands to build using CMake are as follows:

```console
$ mkdir build/ && cd build/
$ cmake ..
$ make -j$(sysctl -n hw.ncpu) # This will use all the cores on your system
```

Next, move back into the root directory and start the game with `./start_mcpelauncher.sh`

## Running
1. Clone this repository
2. Compile the launcher
3. You'll need to obtain a x86 MCPE .apk. If you have built the application with the First Time Setup, you'll be able to log in to your Google account the first time you start the launcher and download it. If you can not do this then after you have downloaded MCPE, run: mcpelauncher extract _filename_
5. Run the launcher! (if you've built in `build`, run `cd .. && ./build/mcpelauncher`)

If the extract script fails with an error about the .apk not being x86, it means that you have provided it a bad .apk.
You'll need to purchase MCPE on Google Play and use the first time setup.

## License and thanks
This project is licensed under GPL, some parts of the sources are under BSD.

This project uses modified versions of Hybris, EGLUT and RapidXML. The proprietary FMOD library is also used for sound support.
