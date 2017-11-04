#pragma once

#include <thread>
#include <list>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>
#include "include/cef_app.h"
#include "include/cef_client.h"

class MyRenderProcessHandler;

class BrowserApp : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler {

private:
    friend class BrowserClient;

    std::thread cefThread;
    std::function<void ()> contextCallback;
    std::unordered_map<int, CefRefPtr<CefBrowser>> renderProcessBrowsers;
    std::unordered_map<int, std::shared_ptr<MyRenderProcessHandler>> browserRenderHandlers;

    static std::unordered_map<std::string, std::function<std::shared_ptr<MyRenderProcessHandler> (CefRefPtr<CefBrowser> browser)>> knownRenderHandlers;

    static void DoCefThread();

public:
    static CefMainArgs cefMainArgs;
    static CefRefPtr<BrowserApp> singleton;

    static void RunWithContext(std::function<void ()> contextCallback);

    template <typename T>
    static void RegisterRenderProcessHandler() {
        knownRenderHandlers[T::Name] = [](CefRefPtr<CefBrowser> browser) { return std::shared_ptr<T>(new T(browser)); };
    }

    static void Shutdown();

    BrowserApp();

    void SetRenderHandler(int browser, std::string const& handler);

    virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }

    virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    // CefBrowserProcessHandler methods:
    virtual void OnContextInitialized() override;

    // CefRenderProcessHandler methods:
    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;

    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;

private:
IMPLEMENT_REFCOUNTING(BrowserApp);

};

class MyRenderProcessHandler {

private:
    CefRefPtr<CefBrowser> browser;

public:
    MyRenderProcessHandler(CefRefPtr<CefBrowser> browser) : browser(browser) {}

    CefRefPtr<CefBrowser> GetBrowser() { return browser; }

    virtual ~MyRenderProcessHandler() { }

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) {}

};

class BrowserClient : public CefClient, public CefLifeSpanHandler, public CefLoadHandler {

private:
    BrowserApp& app;
    std::string renderHandlerId;

public:
    BrowserClient(BrowserApp& app) : app(app) {}
    BrowserClient() : app(*BrowserApp::singleton) { }

    template <typename T>
    void SetRenderHandler() {
        renderHandlerId = T::Name;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                             const CefString& errorText, const CefString& failedUrl) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool forceClose);

    CefRefPtr<CefBrowser> GetPrimaryBrowser() { return browserList.front(); }

protected:

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browserList;

IMPLEMENT_REFCOUNTING(BrowserClient);
};