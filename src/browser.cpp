// Based on the CEF demo library
#include "browser.h"

#include <X11/Xlib.h>
#include <iostream>
#include "common.h"
#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "msa.h"
#include "xboxlive.h"
#include "base64.h"
#include "../mcpe/Xbox.h"

extern "C" {
#include "eglut.h"
}

std::string const XboxLoginBrowserApp::APPEND_URL_PARAMS = "platform=android2.1.0504.0524&client_id=android-app%3A%2F%2Fcom.mojang.minecraftpe.H62DKCBHJP6WXXIV7RBFOGOL4NAK4E6Y&cobrandid=90023&mkt=en-US&phone=&email=";

static int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
    LOG(WARNING) << "X error received: "
                 << "type " << event->type << ", "
                 << "serial " << event->serial << ", "
                 << "error_code " << static_cast<int>(event->error_code) << ", "
                 << "request_code " << static_cast<int>(event->request_code)
                 << ", "
                 << "minor_code " << static_cast<int>(event->minor_code);
    return 0;
}

static int XIOErrorHandlerImpl(Display* display) {
    return 0;
}

CefMainArgs XboxLoginBrowserApp::cefMainArgs;

void XboxLoginBrowserApp::openBrowser(xbox::services::system::user_auth_android* userAuth) {
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    CefRefPtr<XboxLoginBrowserApp> app (new XboxLoginBrowserApp());

    CefSettings settings;
    settings.single_process = true;
    settings.no_sandbox = true;
    CefString(&settings.user_agent) = "Dalvik/2.1.0 (Linux; U; Android 6.0.1; ONEPLUS A3003 Build/MMB29M); com.mojang.minecraftpe/0.15.2.1; MsaAndroidSdk/2.1.0504.0524";
    CefString(&settings.resources_dir_path) = getCWD() + "libs/cef/";
    CefString(&settings.locales_dir_path) = getCWD() + "libs/cef/locales/";
    CefInitialize(cefMainArgs, settings, app.get(), nullptr);
    CefRunMessageLoop();
    CefShutdown();

    if (app->succeeded) {
        using namespace xbox::services::system;
        auth_manager::auth_manager_set_rps_ticket(userAuth->auth_mgr, app->binaryToken);
        auto initTask = auth_manager::auth_manager_initialize_default_nsal(userAuth->auth_mgr);
        auto initRet = pplx::task::task_xbox_live_result_void_get(&initTask);
        if (initRet.code != 0)
            throw std::runtime_error("Failed to initialize default nsal");
        std::vector<token_identity_type> types = {(token_identity_type) 3, (token_identity_type) 1,
                                                  (token_identity_type) 2};
        auto config = auth_manager::auth_manager_get_auth_config(userAuth->auth_mgr);
        auth_config::auth_config_set_xtoken_composition(config.get(), types);
        std::string const& endpoint = auth_config::auth_config_xbox_live_endpoint(config.get());
        printf("Xbox Live Endpoint: %s\n", endpoint.c_str());
        auto task = auth_manager::auth_manager_internal_get_token_and_signature(userAuth->auth_mgr, "GET", endpoint, endpoint, std::string(), std::vector<unsigned char>(), false, false, std::string()); // I'm unsure about the vector (and pretty much only about the vector)
        printf("Get token and signature task started!\n");
        auto ret = pplx::task::task_xbox_live_result_token_and_signature_get(&task);
        printf("User info received! Status: %i\n", ret.code);
        printf("Gamertag = %s, age group = %s, web account id = %s\n", ret.data.gamertag.c_str(), ret.data.age_group.c_str(), ret.data.web_account_id.c_str());

        userAuth->auth_flow->auth_flow_result.code = 0;
        userAuth->auth_flow->auth_flow_result.xbox_user_id = ret.data.xbox_user_id;
        userAuth->auth_flow->auth_flow_result.gamertag = ret.data.gamertag;
        userAuth->auth_flow->auth_flow_result.age_group = ret.data.age_group;
        userAuth->auth_flow->auth_flow_result.privileges = ret.data.privileges;
        userAuth->auth_flow->auth_flow_result.cid = app->cid;
        pplx::task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set(
                &userAuth->auth_flow->auth_flow_event, userAuth->auth_flow->auth_flow_result);
    } else {
        userAuth->auth_flow->auth_flow_result.code = 2;
        pplx::task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set(
                &userAuth->auth_flow->auth_flow_event, userAuth->auth_flow->auth_flow_result);
    }
}

void XboxLoginBrowserApp::ContinueLogIn() {
    std::string username = externalInterfaceHandler->properties["Username"];
    cid = externalInterfaceHandler->properties["CID"];
    std::string daToken = externalInterfaceHandler->properties["DAToken"];
    std::string daTokenKey = externalInterfaceHandler->properties["DASessionKey"];
    std::shared_ptr<MSALegacyToken> token (new MSALegacyToken(daToken, Base64::decode(daTokenKey)));
    std::shared_ptr<MSAAccount> account (new MSAAccount(XboxLiveHelper::getMSALoginManager(), username, cid, token));
    auto ret = account->requestTokens({{"http://Passport.NET/tb"}, {"user.auth.xboxlive.com", "mbi_ssl"}});
    auto xboxLiveToken = ret[{"user.auth.xboxlive.com"}];
    if (xboxLiveToken.hasError()) {
        printf("Has error\n");
        if (xboxLiveToken.getError()->inlineAuthUrl.empty()) {
            handler->CloseAllBrowsers(true);
        } else {
            std::string url = xboxLiveToken.getError()->inlineAuthUrl;
            if (url.find('?') == std::string::npos)
                url = url + "?" + APPEND_URL_PARAMS;
            else
                url = url + "&" + APPEND_URL_PARAMS;
            printf("Navigating to URL: %s\n", url.c_str());
            handler->GetPrimaryBrowser()->GetMainFrame()->LoadURL(url);
        }
        return;
    }
    binaryToken = std::static_pointer_cast<MSACompactToken>(xboxLiveToken.getToken())->getBinaryToken();
    succeeded = true;
    printf("Binary token: %s\n", binaryToken.c_str());
    XboxLiveHelper::getMSAStorageManager()->setAccount(account);
    Close(true);
}

XboxLoginBrowserApp::XboxLoginBrowserApp() : handler(new SimpleHandler()),
                                             externalInterfaceHandler(new XboxLiveV8Handler(*this)) {

}

void XboxLoginBrowserApp::OnContextInitialized() {
    CefWindowInfo window;
    window.width = 480;
    window.height = 640;
    window.x = eglutGetWindowX() + eglutGetWindowWidth() / 2 - window.width / 2;
    window.y = eglutGetWindowY() + eglutGetWindowHeight() / 2 - window.height / 2;
    CefBrowserSettings browserSettings;

    CefBrowserHost::CreateBrowser(window, handler, "https://login.live.com/ppsecure/InlineConnect.srf?id=80604&" + APPEND_URL_PARAMS, browserSettings, NULL);
}

void XboxLoginBrowserApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context) {
    printf("OnContextCreated\n");
    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("Property", CefV8Value::CreateFunction("Property", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalBack", CefV8Value::CreateFunction("FinalBack", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalNext", CefV8Value::CreateFunction("FinalNext", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("external", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

void XboxLoginBrowserApp::Close(bool success) {
    succeeded = success;
    handler->CloseAllBrowsers(true);
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
    CEF_REQUIRE_UI_THREAD();
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    browserList.push_back(browser);
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    CEF_REQUIRE_UI_THREAD();

    for (auto bit = browserList.begin(); bit != browserList.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browserList.erase(bit);
            break;
        }
    }

    if (browserList.empty()) {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                                const CefString& errorText, const CefString& failedUrl) {
    CEF_REQUIRE_UI_THREAD();
    if (errorCode == ERR_ABORTED)
        return;

    std::stringstream ss;
    ss << "<html><body><h2>Failed to load URL " << std::string(failedUrl) << "</h2>"
       << "<p>" << std::string(errorText) << " (" << errorCode << ").</p></body></html>";
    frame->LoadString(ss.str(), failedUrl);
}

void SimpleHandler::CloseAllBrowsers(bool forceClose) {
    if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this, forceClose));
        return;
    }
    for (auto it = browserList.begin(); it != browserList.end(); ++it)
        (*it)->GetHost()->CloseBrowser(forceClose);
}

bool XboxLiveV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "Property") {
        if (args.size() == 1 && args[0]->IsString()) {
            CefString prop = args[0]->GetStringValue();
            printf("Get Property %s\n", prop.ToString().c_str());
            retval = CefV8Value::CreateString(properties[prop]);
            return true;
        } else if (args.size() == 2 && args[0]->IsString() && args[1]->IsString()) {
            CefString prop = args[0]->GetStringValue();
            CefString val = args[1]->GetStringValue();
            printf("Set Property %s = %s\n", prop.ToString().c_str(), val.ToString().c_str());
            properties[prop] = val;
            return true;
        }
    } else if (name == "FinalBack") {
        // Cancel
        app.Close(false);
        return true;
    } else if (name == "FinalNext") {
        // Success!
        app.ContinueLogIn();
        return true;
    }
    printf("Invalid Execute: %s\n", name.ToString().c_str());
    return false;
}