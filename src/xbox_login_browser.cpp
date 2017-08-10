// Based on the CEF demo library
#include "xbox_login_browser.h"

#include <iostream>
#include <include/views/cef_window.h>
#include <X11/Xlib.h>
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
#include "eglut_x11.h"
}

std::string const XboxLoginRenderHandler::Name = "XboxLoginRenderHandler";
std::string const XboxLoginBrowserClient::APPEND_URL_PARAMS = "platform=android2.1.0504.0524&client_id=android-app%3A%2F%2Fcom.mojang.minecraftpe.H62DKCBHJP6WXXIV7RBFOGOL4NAK4E6Y&cobrandid=90023&mkt=en-US&phone=&email=";
AsyncResult<XboxLoginResult> XboxLoginBrowserClient::resultState;

void XboxLoginBrowserClient::OpenBrowser(xbox::services::system::user_auth_android* auth) {
    printf("OpenBrowser\n");

    BrowserApp::RunWithContext([] {
        CefRefPtr<XboxLoginBrowserClient> client = new XboxLoginBrowserClient();

        XWindowAttributes attrs;
        XGetWindowAttributes(eglutGetDisplay(), eglutGetWindowHandle(), &attrs);

        MyWindowDelegate::Options window;
        window.w = 480;
        window.h = 640;
        window.x = eglutGetWindowX() - attrs.x + eglutGetWindowWidth() / 2 - window.w / 2;
        window.y = eglutGetWindowY() - attrs.y + eglutGetWindowHeight() / 2 - window.h / 2;
        window.modal = true;
        window.modalParent = eglutGetWindowHandle();
        CefBrowserSettings browserSettings;
        browserSettings.background_color = 0xFF2A2A2A;
        CefRefPtr<CefBrowserView> view = CefBrowserView::CreateBrowserView(
                client, "https://login.live.com/ppsecure/InlineConnect.srf?id=80604&" + APPEND_URL_PARAMS, browserSettings, NULL, NULL);
        CefWindow::CreateTopLevelWindow(new MyWindowDelegate(view, window));
    });

    resultState.Clear();

    XboxLoginResult result = resultState.Get();
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

XboxLoginBrowserClient::XboxLoginBrowserClient() {
    SetRenderHandler<XboxLoginRenderHandler>();
}

void XboxLoginBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    BrowserClient::OnBeforeClose(browser);
    if (browserList.empty()) {
        XboxLoginResult result;
        result.success = false;
        resultState.Set(result);
    }
}

void XboxLoginBrowserClient::ContinueLogIn(std::string const& username, std::string const& cid,
                                           std::string const& daToken, std::string const& daTokenKey) {
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
                url = url + "?" + XboxLoginBrowserClient::APPEND_URL_PARAMS;
            else
                url = url + "&" + XboxLoginBrowserClient::APPEND_URL_PARAMS;
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
    resultState.Set(result);
    CloseAllBrowsers(true);
}

void XboxLoginBrowserClient::CancelLogIn() {
    CloseAllBrowsers(true);
}

bool XboxLoginBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                      CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "ContinueLogIn") {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        ContinueLogIn(args->GetString(0), args->GetString(1), args->GetString(2), args->GetString(3));
        return true;
    } else if (message->GetName() == "CancelLogIn") {
        CancelLogIn();
        return true;
    }
    return false;
}

void XboxLoginRenderHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                              CefRefPtr<CefV8Context> context) {
    printf("XboxLoginBrowserHandler::OnContextCreated\n");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new XboxLiveV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("Property", CefV8Value::CreateFunction("Property", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalBack", CefV8Value::CreateFunction("FinalBack", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("FinalNext", CefV8Value::CreateFunction("FinalNext", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("external", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

void XboxLoginRenderHandler::RequestContinueLogIn(std::map<CefString, CefString> const& properties) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("ContinueLogIn");
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    args->SetString(0, properties.at("Username"));
    args->SetString(1, properties.at("CID"));
    args->SetString(2, properties.at("DAToken"));
    args->SetString(3, properties.at("DASessionKey"));
    GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
}

void XboxLoginRenderHandler::RequestCancelLogIn() {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CancelLogIn");
    GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
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
        handler.RequestCancelLogIn();
        return true;
    } else if (name == "FinalNext") {
        // Success!
        handler.RequestContinueLogIn(properties);
        return true;
    }
    printf("Invalid Execute: %s\n", name.ToString().c_str());
    return false;
}