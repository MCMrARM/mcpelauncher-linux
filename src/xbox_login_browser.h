#pragma once

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "minecraft/Xbox.h"
#include "browser.h"
#include "async_result_util.h"

class XboxLoginRenderHandler;
class XboxLiveV8Handler;

struct XboxLoginResult {
    bool success = false;
    std::string binaryToken;
    std::string cid;
};

class XboxLoginBrowserClient : public BrowserClient {

private:

    static AsyncResult<XboxLoginResult> resultState;

public:

    static const std::string APPEND_URL_PARAMS;

    static void OpenBrowser(xbox::services::system::user_auth_android* userAuth);

    XboxLoginBrowserClient();

    // CefLifeSpanHandler methods:
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // Message handling:
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    void ContinueLogIn(std::string const& username, std::string const& cid, std::string const& daToken,
                       std::string const& daTokenKey);

    void CancelLogIn();
};

class XboxLoginRenderHandler : public MyRenderProcessHandler {

private:
    CefRefPtr<XboxLiveV8Handler> externalInterfaceHandler;

public:
    XboxLoginRenderHandler(CefRefPtr<CefBrowser> browser) : MyRenderProcessHandler(browser) {}

    static const std::string Name;

    void RequestContinueLogIn(std::map<CefString, CefString> const& properties);
    void RequestCancelLogIn();


    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

};

class XboxLiveV8Handler : public CefV8Handler {

private:
    XboxLoginRenderHandler& handler;

public:
    std::map<CefString, CefString> properties;

    XboxLiveV8Handler(XboxLoginRenderHandler& handler) : handler(handler) {}

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
IMPLEMENT_REFCOUNTING(XboxLiveV8Handler);
};