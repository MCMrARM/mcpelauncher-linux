#!/usr/bin/env bash
if [ "$(uname)" == "Darwin" ]; then
  echo 'CEF is not supported on macOS at this time'
  exit -1
fi

CEF_DOWNLOAD_URL="http://opensource.spotify.com/cefbuilds/cef_binary_3.3112.1653.gf69054f_linux32_minimal.tar.bz2"
CEF_PATH="cef.tar.bz2"
CEF_TMP_DIR="tmp_cef"
TARGET_BASE_DIR="${PWD}/libs/cef"
CPU_CORE_COUNT=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)
CPU_CORE_COUNT=$(($CPU_CORE_COUNT+1))

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
TARGET_RESOURCES="${TARGET_BASE_DIR}/res"
TARGET_LIBRARIES="${TARGET_BASE_DIR}/lib"
TARGET_INCLUDE="${TARGET_BASE_DIR}/include"
TARGET_LOCALES="${TARGET_RESOURCES}/locales"
TARGET_APPLICATION="${TARGET_BASE_DIR}/bin"
rm -rf "${TARGET_BASE_DIR}"
mkdir -p "${TARGET_RESOURCES}"
mkdir -p "${TARGET_LIBRARIES}"
mkdir -p "${TARGET_INCLUDE}"
mkdir -p "${TARGET_LOCALES}"
mkdir -p "${TARGET_APPLICATION}"

cp -r include/. ${TARGET_INCLUDE}

cd Resources/
cp cef.pak cef_100_percent.pak cef_200_percent.pak cef_extensions.pak "${TARGET_RESOURCES}"
cp -r locales "${TARGET_RESOURCES}"
cp icudtl.dat "${TARGET_APPLICATION}"
cd ..

cd Release
cp libcef.so "${TARGET_LIBRARIES}"
cp natives_blob.bin snapshot_blob.bin "${TARGET_APPLICATION}"
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
