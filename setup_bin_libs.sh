#!/usr/bin/env bash
BIN_LIBS_DOWNLOAD_URL="https://github.com/MCMrARM/mcpelauncher-linux-bin/archive/a8ea807aa1109ab7cadafdc40dc7720f14f3389f.zip"
BIN_LIBS_PATH="bin_libs.zip"
BIN_LIBS_TMP_DIR="tmp_binlibs"

# Download
echo "Downloading: ${BIN_LIBS_DOWNLOAD_URL}"
wget -O "${BIN_LIBS_PATH}" -c "${BIN_LIBS_DOWNLOAD_URL}"

# Extract
echo "Extracting"
rm -rf "${BIN_LIBS_TMP_DIR}"
mkdir -p "${BIN_LIBS_TMP_DIR}"
unzip "${BIN_LIBS_PATH}" -d "${BIN_LIBS_TMP_DIR}"

# Install
echo "Moving files around"
cd "${BIN_LIBS_TMP_DIR}"/*
cp -r * ../../
cd ../..

# Clean up
echo "Cleaning up"
rm -rf "${BIN_LIBS_TMP_DIR}"
rm "${BIN_LIBS_PATH}"