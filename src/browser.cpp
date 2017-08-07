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
#include "minecraft/Xbox.h"

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
CefRefPtr<XboxLoginBrowserApp> XboxLoginBrowserApp::singleton (new XboxLoginBrowserApp());

void XboxLoginBrowserApp::doCefThread() {
    CefSettings settings;
    settings.no_sandbox = true;
    CefString(&settings.user_agent) = "Dalvik/2.1.0 (Linux; U; Android 6.0.1; ONEPLUS A3003 Build/MMB29M); com.mojang.minecraftpe/0.15.2.1; MsaAndroidSdk/2.1.0504.0524";
    CefString(&settings.resources_dir_path) = getCWD() + "libs/cef/";
    CefString(&settings.locales_dir_path) = getCWD() + "libs/cef/locales/";
    CefInitialize(cefMainArgs, settings, singleton.get(), nullptr);
    CefRunMessageLoop();
    CefShutdown();
}

void XboxLoginBrowserApp::OpenBrowser(xbox::services::system::user_auth_android* auth) {
    XSetErrorHandler(XErrorHandlerImpl);
    XSetIOErrorHandler(XIOErrorHandlerImpl);

    printf("OpenBrowser\n");

    singleton->ClearResult();

    if (!singleton->cefThread.joinable())
        singleton->cefThread = std::thread(doCefThread);
    else
        singleton->OnContextInitialized();

    XboxLoginResult result = singleton->GetResult();
    if (result.success) {
        XboxLiveHelper::invokeXbLogin(auth, result.binaryToken);
        auth->auth_flow->auth_flow_result.code = 0;
        auth->auth_flow->auth_flow_result.cid = result.cid;
        pplx::task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set(
                &auth->auth_flow->auth_flow_event, auth->auth_flow->auth_flow_result);
    } else {
        auth->auth_flow->auth_flow_result.code = 2;
        pplx::task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set(
                &auth->auth_flow->auth_flow_event, auth->auth_flow->auth_flow_result);
    }
}

void XboxLoginBrowserApp::Shutdown() {
    if (singleton->cefThread.joinable()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("Shutdown");
        singleton->handler->GetPrimaryBrowser()->SendProcessMessage(PID_BROWSER, msg);

        singleton->cefThread.join();
    }
}

XboxLoginBrowserApp::XboxLoginBrowserApp() {

}

void XboxLoginBrowserApp::OnContextInitialized() {
    handler = new SimpleHandler(*this);

    CefWindowInfo window;
    window.width = 480;
    window.height = 640;
    window.x = eglutGetWindowX() + eglutGetWindowWidth() / 2 - window.width / 2;
    window.y = eglutGetWindowY() + eglutGetWindowHeight() / 2 - window.height / 2;
    CefBrowserSettings browserSettings;
    browserSettings.background_color = 0xFF2A2A2A;
    CefBrowserHost::CreateBrowser(window, handler, "https://login.live.com/ppsecure/InlineConnect.srf?id=80604&" + APPEND_URL_PARAMS, browserSettings, NULL);
}

void XboxLoginBrowserApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                           CefRefPtr<CefV8Context> context) {
    printf("OnContextCreated\n");
    primaryBrowser = browser;

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new XboxLiveV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("Property", CefV8Value::CreateFunction("Property", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalBack", CefV8Value::CreateFunction("FinalBack", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalNext", CefV8Value::CreateFunction("FinalNext", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("external", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

void XboxLoginBrowserApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                            CefRefPtr<CefV8Context> context) {
    primaryBrowser = nullptr;
}

void XboxLoginBrowserApp::RequestContinueLogIn(std::map<CefString, CefString> const& properties) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("ContinueLogIn");
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    args->SetString(0, properties.at("Username"));
    args->SetString(1, properties.at("CID"));
    args->SetString(2, properties.at("DAToken"));
    args->SetString(3, properties.at("DASessionKey"));
    primaryBrowser->SendProcessMessage(PID_BROWSER, msg);
}

void XboxLoginBrowserApp::RequestCancelLogIn() {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CancelLogIn");
    primaryBrowser->SendProcessMessage(PID_BROWSER, msg);
}

XboxLoginResult XboxLoginBrowserApp::GetResult() {
    std::unique_lock<std::mutex> lock(resultMutex);
    resultConditionVariable.wait(lock, [this] { return hasResult; });
    return result;
}

void XboxLoginBrowserApp::SetResult(XboxLoginResult const& result) {
    std::unique_lock<std::mutex> lock(resultMutex);
    this->result = result;
    hasResult = true;
    lock.unlock();
    resultConditionVariable.notify_all();
    lock.lock();
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    browserList.push_back(browser);
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    for (auto bit = browserList.begin(); bit != browserList.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browserList.erase(bit);
            break;
        }
    }

    if (browserList.empty()) {
        XboxLoginResult result;
        result.success = false;
        app.SetResult(result);
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                                const CefString& errorText, const CefString& failedUrl) {
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


void SimpleHandler::ContinueLogIn(std::string const& username, std::string const& cid, std::string const& daToken,
                                        std::string const& daTokenKey) {
    std::shared_ptr<MSALegacyToken> token (new MSALegacyToken(daToken, Base64::decode(daTokenKey)));
    std::shared_ptr<MSAAccount> account (new MSAAccount(XboxLiveHelper::getMSALoginManager(), username, cid, token));
    auto ret = account->requestTokens({{"http://Passport.NET/tb"}, {"user.auth.xboxlive.com", "mbi_ssl"}});
    auto xboxLiveToken = ret[{"user.auth.xboxlive.com"}];
    if (xboxLiveToken.hasError()) {
        printf("Has error\n");
        if (xboxLiveToken.getError()->inlineAuthUrl.empty()) {
            CloseAllBrowsers(true);
        } else {
            std::string url = xboxLiveToken.getError()->inlineAuthUrl;
            if (url.find('?') == std::string::npos)
                url = url + "?" + XboxLoginBrowserApp::APPEND_URL_PARAMS;
            else
                url = url + "&" + XboxLoginBrowserApp::APPEND_URL_PARAMS;
            printf("Navigating to URL: %s\n", url.c_str());
            GetPrimaryBrowser()->GetMainFrame()->LoadURL(url);
        }
        return;
    }
    XboxLoginResult result;
    result.success = true;
    result.binaryToken = std::static_pointer_cast<MSACompactToken>(xboxLiveToken.getToken())->getBinaryToken();
    result.cid = cid;
    printf("Binary token: %s\n", result.binaryToken.c_str());
    XboxLiveHelper::getMSAStorageManager()->setAccount(account);
    app.SetResult(result);
    CloseAllBrowsers(true);
}

void SimpleHandler::CancelLogIn() {
    CloseAllBrowsers(true);
}

bool SimpleHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                             CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "ContinueLogIn") {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        ContinueLogIn(args->GetString(0), args->GetString(1), args->GetString(2), args->GetString(3));
        return true;
    } else if (message->GetName() == "CancelLogIn") {
        CancelLogIn();
        return true;
    } else if (message->GetName() == "Shutdown") {
        CefQuitMessageLoop();
        return true;
    }
    return false;
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
        app.RequestCancelLogIn();
        return true;
    } else if (name == "FinalNext") {
        // Success!
        app.RequestContinueLogIn(properties);
        return true;
    }
    printf("Invalid Execute: %s\n", name.ToString().c_str());
    return false;
}