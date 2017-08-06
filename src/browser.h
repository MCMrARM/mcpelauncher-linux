#pragma once

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "../mcpe/Xbox.h"

class SimpleHandler;
class XboxLiveV8Handler;

struct XboxLoginResult {
    bool success = false;
    std::string binaryToken;
    std::string cid;
};

class XboxLoginBrowserApp : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {

private:

    CefRefPtr<SimpleHandler> handler;
    CefRefPtr<XboxLiveV8Handler> externalInterfaceHandler;
    CefRefPtr<CefBrowser> primaryBrowser;
    std::thread cefThread;
    std::mutex resultMutex;
    std::condition_variable resultConditionVariable;
    bool hasResult = false;
    XboxLoginResult result;

    static void doCefThread();

public:

    static const std::string APPEND_URL_PARAMS;

    static CefMainArgs cefMainArgs;
    static CefRefPtr<XboxLoginBrowserApp> singleton;

    static void OpenBrowser(xbox::services::system::user_auth_android* userAuth);

    static void Shutdown();

    XboxLoginBrowserApp();

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    virtual void OnContextInitialized() override;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                   CefRefPtr<CefV8Context> context) override;

    void RequestContinueLogIn(std::map<CefString, CefString> const& properties);

    void RequestCancelLogIn();

    XboxLoginResult GetResult();
    void SetResult(XboxLoginResult const& result);
    void ClearResult() {
        resultMutex.lock();
        hasResult = false;
        resultMutex.unlock();
    }


private:
IMPLEMENT_REFCOUNTING(XboxLoginBrowserApp);

};

class SimpleHandler : public CefClient, public CefLifeSpanHandler, public CefLoadHandler {

private:
    XboxLoginBrowserApp& app;

public:
    SimpleHandler(XboxLoginBrowserApp& app) : app(app) {}

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                             const CefString& errorText, const CefString& failedUrl) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool forceClose);

    CefRefPtr<CefBrowser> GetPrimaryBrowser() { return browserList.front(); }

    void ContinueLogIn(std::string const& username, std::string const& cid, std::string const& daToken,
                       std::string const& daTokenKey);

    void CancelLogIn();

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