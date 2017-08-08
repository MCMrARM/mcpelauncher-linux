#pragma once

#include <string>
#include <playapi/util/config.h>

class GooglePlayHelper {

private:
    static std::string const DOWNLOAD_PACKAGE;

    playapi::config config;

public:
    bool handleLoginAndApkDownload();

};