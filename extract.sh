#!/usr/bin/env bash

if [ -z ${1} ]; then
    echo "Please specify the archive"
    exit
fi
if [ ! -f ${1} ]; then
    echo "The specified file doesn't exist!"
    exit
fi
if ! [[ ${1} =~ \.apk$ ]]; then
    echo "The file must be an .apk"
    exit
fi

rm -rf assets/

unzip "${1}" "assets/*"
unzip -p "${1}" lib/x86/libminecraftpe.so > libs/libminecraftpe.so
unzip -p "${1}" res/raw/xboxservices.config > assets/xboxservices.config
