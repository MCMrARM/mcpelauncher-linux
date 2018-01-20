#pragma once

#include <string>
#include <thread>
#include <playapi/util/config.h>
#include <include/cef_client.h>
#include "../common/browser.h"

class InitialSetupBrowserClient;

class GooglePlayHelper {

private:
    static std::string const DOWNLOAD_PACKAGE;

    playapi::config config;

public:
    static GooglePlayHelper singleton;

    bool handleLoginAndApkDownloadSync(InitialSetupBrowserClient* setup, MyWindowDelegate::Options const& windowInfo);

    void handleLoginAndApkDownload(InitialSetupBrowserClient* setup, MyWindowDelegate::Options const& windowInfo);

};