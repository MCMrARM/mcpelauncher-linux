#include "google_play_helper.h"
#include "google_login_browser.h"
#include <fstream>
#include <playapi/api.h>
#include <playapi/login.h>
#include <playapi/device_info.h>
#include <zlib.h>
#include <iostream>
#include "../../gplay_api/src/config.h"
#include "initial_setup_browser.h"
#include "../common/extract.h"
#include "../common/path_helper.h"

std::string const GooglePlayHelper::DOWNLOAD_PACKAGE = "com.mojang.minecraftpe";

GooglePlayHelper GooglePlayHelper::singleton;

static void do_zlib_inflate(z_stream& zs, FILE* file, char* data, size_t len, int flags) {
    char buf[4096];
    int ret;
    zs.avail_in = (uInt) len;
    zs.next_in = (unsigned char*) data;
    zs.avail_out = 0;
    while (zs.avail_out == 0) {
        zs.avail_out = 4096;
        zs.next_out = (unsigned char*) buf;
        ret = inflate(&zs, flags);
        assert(ret != Z_STREAM_ERROR);
        fwrite(buf, 1, sizeof(buf) - zs.avail_out, file);
    }
}

bool GooglePlayHelper::handleLoginAndApkDownloadSync(InitialSetupBrowserClient* setup,
                                                     MyWindowDelegate::Options const& windowInfo) {
    app_config conf (PathHelper::getPrimaryDataDirectory() + "playdl.conf");
    conf.load();

    std::string device_path = PathHelper::getPrimaryDataDirectory() + "device.conf";
    bool should_save = true;

    playapi::device_info device;
    {
        std::ifstream dev_info_file(device_path);
        playapi::config dev_info_conf;
        dev_info_conf.load(dev_info_file);
        device.load(dev_info_conf);
    }
    device_config dev_state(device_path + ".state");
    dev_state.load();
    dev_state.load_device_info_data(device);
    device.generate_fields();
    dev_state.set_device_info_data(device);
    dev_state.save();

    playapi::login_api login(device);
    login.set_checkin_data(dev_state.checkin_data);
    if (conf.user_token.empty()) {
        auto result = GoogleLoginBrowserClient::OpenBrowser(windowInfo);
        if (!result.success) {
            setup->NotifyApkSetupResult(false);
            return false;
        }
        login.perform_with_access_token(result.oauthToken, result.email, true);
        should_save = setup->AskYesNo("Store authentication", "Would you like to save the authentication information for future usage (eg. downloading newer version of Minecraft)?").Get();
        conf.user_email = login.get_email();
        conf.user_token = login.get_token();
        if (should_save)
            conf.save();
    } else {
        login.set_token(conf.user_email, conf.user_token);
        login.verify();
    }
    if (dev_state.checkin_data.android_id == 0) {
        playapi::checkin_api checkin(device);
        checkin.add_auth(login);
        dev_state.checkin_data = checkin.perform_checkin();
        dev_state.save();
    }

    playapi::api play(device);
    play.set_auth(login);
    play.set_checkin_data(dev_state.checkin_data);
    dev_state.load_api_data(login.get_email(), play);
    if (play.toc_cookie.length() == 0 || play.device_config_token.length() == 0) {
        play.fetch_user_settings();
        auto toc = play.fetch_toc();
        if (toc.payload().tocresponse().has_cookie())
            play.toc_cookie = toc.payload().tocresponse().cookie();

        if (play.fetch_toc().payload().tocresponse().requiresuploaddeviceconfig()) {
            auto resp = play.upload_device_config();
            play.device_config_token = resp.payload().uploaddeviceconfigresponse().uploaddeviceconfigtoken();

            toc = play.fetch_toc();
            assert(!toc.payload().tocresponse().requiresuploaddeviceconfig() &&
                   toc.payload().tocresponse().has_cookie());
            play.toc_cookie = toc.payload().tocresponse().cookie();
            if (toc.payload().tocresponse().has_toscontent() && toc.payload().tocresponse().has_tostoken()) {
                InitialSetupBrowserClient::AskTosResult accepted = setup->AskAcceptTos(toc.payload().tocresponse().toscontent()).Get();
                if (accepted == InitialSetupBrowserClient::AskTosResult::DECLINED) {
                    return false;
                }
                auto tos = play.accept_tos(toc.payload().tocresponse().tostoken(), accepted == InitialSetupBrowserClient::AskTosResult::ACCEPTED_MARKETING);
                assert(tos.payload().has_accepttosresponse());
                dev_state.set_api_data(login.get_email(), play);
                dev_state.save();
            }
        }
    }
    auto details = play.details(DOWNLOAD_PACKAGE).payload().detailsresponse().docv2();
    auto resp = play.delivery(DOWNLOAD_PACKAGE, details.details().appdetails().versioncode(), std::string());
    auto dd = resp.payload().deliveryresponse().appdeliverydata();
    playapi::http_request req(dd.gzippeddownloadurl());
    req.set_encoding("gzip,deflate");
    req.add_header("Accept-Encoding", "identity");
    auto cookie = dd.downloadauthcookie(0);
    req.add_header("Cookie", cookie.name() + "=" + cookie.value());
    req.set_user_agent("AndroidDownloadManager/" + device.build_version_string + " (Linux; U; Android " +
                       device.build_version_string + "; " + device.build_model + " Build/" + device.build_id + ")");
    req.set_follow_location(true);
    req.set_timeout(0L);

    std::string file_name = DOWNLOAD_PACKAGE + " " + std::to_string(details.details().appdetails().versioncode()) + ".apk";
    FILE* file = fopen(file_name.c_str(), "w");
    z_stream zs;
    zs.zalloc = Z_NULL;
    zs.zfree = Z_NULL;
    zs.opaque = Z_NULL;
    int ret = inflateInit2(&zs, 31);
    assert(ret == Z_OK);

    req.set_custom_output_func([file, &zs](char* data, size_t size) {
        do_zlib_inflate(zs, file, data, size, Z_NO_FLUSH);
        return size;
    });

    curl_off_t old_dlnow = -1LL;
    req.set_progress_callback([&req, &old_dlnow, setup](curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        if (dltotal > 0 && (dlnow >= old_dlnow + 1024L * 100L || dlnow == 0 || dlnow == dltotal))
            setup->NotifyDownloadStatus(true, dlnow, dltotal);
    });
    setup->NotifyDownloadStatus(true, 0LL, 0LL);
    req.perform();

    do_zlib_inflate(zs, file, Z_NULL, 0, Z_FINISH);
    inflateEnd(&zs);

    fclose(file);

    setup->NotifyDownloadStatus(false, 0LL, 0LL);

    ExtractHelper::extractApk(file_name);

    setup->NotifyApkSetupResult(true);
    return true;
}

void GooglePlayHelper::handleLoginAndApkDownload(InitialSetupBrowserClient* setup,
                                                 MyWindowDelegate::Options const& windowInfo) {
    std::thread thread(std::bind(&GooglePlayHelper::handleLoginAndApkDownloadSync, this, setup, windowInfo));
    thread.detach();
}