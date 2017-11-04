#!/usr/bin/env bash
CEF_DOWNLOAD_URL="http://opensource.spotify.com/cefbuilds/cef_binary_3.3112.1653.gf69054f_linux32_minimal.tar.bz2"
CEF_PATH="cef.tar.bz2"
CEF_TMP_DIR="tmp_cef"
TARGET_BASE_DIR="${PWD}"
CPU_CORE_COUNT=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)

# Download
echo "Downloading: ${CEF_DOWNLOAD_URL}"
wget -O "${CEF_PATH}" -c "${CEF_DOWNLOAD_URL}"

# Extract
echo "Extracting"
rm -rf "${CEF_TMP_DIR}"
mkdir -p "${CEF_TMP_DIR}"
tar xvjf "${CEF_PATH}" -C "${CEF_TMP_DIR}"

# Patch CMakeLists.txt so that it compiles
echo "Patching CMakeLists.txt"
cd "${CEF_TMP_DIR}"/*
mv CMakeLists.txt CMakeLists_original.txt
sed 's/^add_subdirectory(tests\//#&/' CMakeLists_original.txt > CMakeLists.txt

# Build
echo "Building"
mkdir -p "build"
cd build
cmake -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 ..
make -j${CPU_CORE_COUNT}
cd ..

# Install
echo "Moving files around"
TARGET_RESOURCES="${TARGET_BASE_DIR}/libs/cef"
TARGET_LIBRARIES="${TARGET_RESOURCES}"
TARGET_INCLUDE="${TARGET_RESOURCES}/include"
TARGET_LOCALES="${TARGET_RESOURCES}/locales"
TARGET_RUNTIME="${TARGET_RESOURCES}/runtime"
echo "Target: ${TARGET_RESOURCES}"
rm -rf "${TARGET_RESOURCES}"
mkdir -p "${TARGET_RESOURCES}"
mkdir -p "${TARGET_INCLUDE}"
mkdir -p "${TARGET_LOCALES}"
mkdir -p "${TARGET_RUNTIME}"

cp -r include/. ${TARGET_INCLUDE}

cd Resources/
cp cef.pak cef_100_percent.pak cef_200_percent.pak cef_extensions.pak "${TARGET_RESOURCES}"
cp -r locales "${TARGET_RESOURCES}"
cp icudtl.dat "${TARGET_RUNTIME}"
cd ..

cd Release
cp libcef.so "${TARGET_LIBRARIES}"
cp natives_blob.bin snapshot_blob.bin "${TARGET_RUNTIME}"
cd ..

cd build/libcef_dll_wrapper
cp libcef_dll_wrapper.a "${TARGET_LIBRARIES}"
cd ../..

# Reduce libcef.so size
echo "Reducing libcef.so size"
strip "${TARGET_LIBRARIES}/libcef.so"

# Clean up
echo "Cleaning up"
cd ../..
rm -rf "${CEF_TMP_DIR}"
rm "${CEF_PATH}"
