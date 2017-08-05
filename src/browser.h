#pragma once

#include <list>
#include <map>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "../mcpe/Xbox.h"

class SimpleHandler;
class XboxLiveV8Handler;

class XboxLoginBrowserApp : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {

private:
    static const std::string APPEND_URL_PARAMS;

    CefRefPtr<SimpleHandler> handler;
    CefRefPtr<XboxLiveV8Handler> externalInterfaceHandler;
    bool succeeded = false;
    std::string binaryToken;

public:
    static CefMainArgs cefMainArgs;

    static void openBrowser(xbox::services::system::user_auth_android* userAuth);

    XboxLoginBrowserApp();

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    virtual void OnContextInitialized() override;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    void Close(bool success);

    void ContinueLogIn();


private:
IMPLEMENT_REFCOUNTING(XboxLoginBrowserApp);

};

class SimpleHandler : public CefClient, public CefDisplayHandler, public CefLifeSpanHandler, public CefLoadHandler {

public:
    SimpleHandler() {}

    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    // CefDisplayHandler methods:
    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
                               const CefString& title) override;

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                             const CefString& errorText, const CefString& failedUrl) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool forceClose);

    CefRefPtr<CefBrowser> GetPrimaryBrowser() { return browserList.front(); }

private:

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browserList;

    // Include the default reference counting implementation.
IMPLEMENT_REFCOUNTING(SimpleHandler);
};

class XboxLiveV8Handler : public CefV8Handler {

private:
    XboxLoginBrowserApp& app;

public:
    std::map<CefString, CefString> properties;

    XboxLiveV8Handler(XboxLoginBrowserApp& app) : app(app) {}

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
IMPLEMENT_REFCOUNTING(XboxLiveV8Handler);
};