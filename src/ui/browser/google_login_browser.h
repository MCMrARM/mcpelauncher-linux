#pragma once

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "browser.h"
#include "../../common/async_result_util.h"

class GoogleLoginRenderHandler;
class GoogleLoginV8Handler;

struct GoogleLoginResult {
    bool success = false;
    std::string email;
    std::string oauthToken;
    std::string userId;
};

class GoogleLoginBrowserClient : public BrowserClient, public CefCookieVisitor {

private:
    AsyncResult<GoogleLoginResult> resultState;

    GoogleLoginResult result;

public:
    static GoogleLoginResult OpenBrowser(MyWindowDelegate::Options const& windowInfo);

    GoogleLoginBrowserClient();

    // CefLifeSpanHandler methods:
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // Message handling:
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    virtual void OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url) override;

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override {}

    virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) override;
};

class DeleteCookieVisitor : public CefCookieVisitor {

public:
    virtual bool Visit(const CefCookie& cookie, int count, int total, bool& deleteCookie) override {
        deleteCookie = true;
    }

private:
IMPLEMENT_REFCOUNTING(DeleteCookieVisitor);

};

class GoogleLoginRenderHandler : public MyRenderProcessHandler {

private:
    CefRefPtr<GoogleLoginV8Handler> externalInterfaceHandler;

public:
    GoogleLoginRenderHandler(CefRefPtr<CefBrowser> browser) : MyRenderProcessHandler(browser) {}

    static const std::string Name;


    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

};

class GoogleLoginV8Handler : public CefV8Handler {

private:
    GoogleLoginRenderHandler& handler;

public:
    GoogleLoginV8Handler(GoogleLoginRenderHandler& handler) : handler(handler) {}

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
IMPLEMENT_REFCOUNTING(GoogleLoginV8Handler);
};