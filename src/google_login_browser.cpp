#include <codecvt>
#include <locale>
#include "google_login_browser.h"

AsyncResult<GoogleLoginResult> GoogleLoginBrowserClient::resultState;
std::string const GoogleLoginRenderHandler::Name = "GoogleLoginRenderHandler";

GoogleLoginResult GoogleLoginBrowserClient::OpenBrowser() {
    printf("GoogleLoginBrowserClient::OpenBrowser\n");

    BrowserApp::RunWithContext([] {
        CefRefPtr<GoogleLoginBrowserClient> client = new GoogleLoginBrowserClient();

        CefWindowInfo window;
        window.width = 480;
        window.height = 640;
        CefBrowserSettings browserSettings;
        CefBrowserHost::CreateBrowser(window, client, "https://accounts.google.com/embedded/setup/v2/android?source=com.android.settings&xoauth_display_name=Android%20Phone&canFrp=1&canSk=1&lang=en&langCountry=en_us&hl=en-US&cc=us", browserSettings, NULL);
    });

    resultState.Clear();
    return resultState.Get();
}

GoogleLoginBrowserClient::GoogleLoginBrowserClient() {
    SetRenderHandler<GoogleLoginRenderHandler>();
}

void GoogleLoginBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    BrowserClient::OnBeforeClose(browser);
    if (browserList.empty())
        resultState.Set(GoogleLoginResult());
}

bool GoogleLoginBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                        CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "SetAccountIdentifier") {
        printf("SetAccountIdentifier\n");
        result.email = message->GetArgumentList()->GetString(0).ToString();
        return true;
    }
    return false;
}

void GoogleLoginBrowserClient::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                               const CefString& url) {
    std::string urlStr = url.ToString();
    auto iof = urlStr.find('?');
    if (iof != std::string::npos)
        urlStr = urlStr.substr(0, iof);
    printf("OnAddressChange %s %i\n", urlStr.c_str(), frame->IsMain());
    CefRefPtr<CefCookieManager> manager = CefCookieManager::GetGlobalManager(NULL);
    manager->VisitAllCookies(this);
}

bool GoogleLoginBrowserClient::Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) {
    CefString cookieName (&cookie.name);
    if (cookieName == "oauth_token") {
        printf("Cookie: oauth_token\n");
        result.oauthToken = CefString(&cookie.value).ToString();
    } else if (cookieName == "user_id") {
        printf("Cookie: user_id\n");
        result.userId = CefString(&cookie.value).ToString();
    }
    if (!result.email.empty() && !result.userId.empty() && !result.oauthToken.empty()) {
        result.success = true;
        resultState.Set(result);
        result.success = false;
        result.email = result.userId = result.oauthToken = std::string();
        CloseAllBrowsers(true);
        return false;
    }
    return true;
}

void GoogleLoginRenderHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                CefRefPtr<CefV8Context> context) {
    printf("GoogleLoginRenderHandler::OnContextCreated\n");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new GoogleLoginV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("log", CefV8Value::CreateFunction("log", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setAccountIdentifier", CefV8Value::CreateFunction("setAccountIdentifier", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("mm", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool GoogleLoginV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                       CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "log" && args.size() == 1 && args[0]->IsString()) {
        CefString prop = args[0]->GetStringValue();
        printf("log: %s\n", prop.ToString().c_str());
        return true;
    }
    if (name == "setAccountIdentifier" && args.size() == 1 && args[0]->IsString()) {
        CefString prop = args[0]->GetStringValue();
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetAccountIdentifier");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetString(0, prop);
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    printf("Invalid Execute: %s\n", name.ToString().c_str());
    return true;
}