#!/usr/bin/env bash
PROTOBUF_DOWNLOAD_URL="https://github.com/google/protobuf/releases/download/v3.3.0/protobuf-cpp-3.3.0.tar.gz"
PROTOBUF_PATH="protobuf.tar.gz"
PROTOBUF_TMP_DIR="tmp_protobuf"
TARGET_BASE_DIR="${PWD}/libs/protobuf"
CPU_CORE_COUNT=$(cat /proc/cpuinfo | awk '/^processor/{print $3}' | tail -1)

# Download
echo "Downloading: ${PROTOBUF_DOWNLOAD_URL}"
wget -O "${PROTOBUF_PATH}" -c "${PROTOBUF_DOWNLOAD_URL}"

# Extract
echo "Extracting"
rm -rf "${PROTOBUF_TMP_DIR}"
mkdir -p "${PROTOBUF_TMP_DIR}"
tar xvzf "${PROTOBUF_PATH}" -C "${PROTOBUF_TMP_DIR}"

cd "${PROTOBUF_TMP_DIR}"/*

# Build
echo "Building"
rm -rf "${TARGET_BASE_DIR}"
./configure \
    --build=i386-pc-linux-gnu \
    --prefix=${TARGET_BASE_DIR} \
    CFLAGS="-m32 -DNDEBUG" \
    CXXFLAGS="-m32 -DNDEBUG -D_GLIBCXX_USE_CXX11_ABI=0" \
    LDFLAGS=-m32
make -j${CPU_CORE_COUNT}
make install

# Remove *.a and *.la
echo "Removing unneeded files"
rm "${TARGET_BASE_DIR}"/lib/*.a

# Clean up
echo "Cleaning up"
cd ../..
rm -rf "${PROTOBUF_TMP_DIR}"
rm "${PROTOBUF_PATH}"
