#pragma once

#include <thread>
#include <list>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <memory>
#include <X11/X.h>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/views/cef_window_delegate.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"

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

    static void RunOnUI(std::function<void ()> function);

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

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) {
        return false;
    }

};

class BrowserClient : public CefClient, public CefLifeSpanHandler, public CefLoadHandler, public CefDisplayHandler {

private:
    BrowserApp& app;
    std::string renderHandlerId;
    CefRefPtr<CefWindow> window;

public:
    BrowserClient(BrowserApp& app) : app(app) {}
    BrowserClient() : app(*BrowserApp::singleton) { }

    template <typename T>
    void SetRenderHandler() {
        renderHandlerId = T::Name;
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }

    virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
                               const CefString& target_frame_name, WindowOpenDisposition target_disposition,
                               bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
                               CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access) override;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                             const CefString& errorText, const CefString& failedUrl) override;

    // CefDisplayHandler methods:
    virtual void OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) override;

    virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool forceClose);

    CefRefPtr<CefBrowser> GetPrimaryBrowser() { return browserList.front(); }

    CefRefPtr<CefWindow> GetPrimaryWindow() { return window; }

protected:

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browserList;

    void SetPrimaryWindow(CefRefPtr<CefWindow> window) {
        this->window = window;
    }

IMPLEMENT_REFCOUNTING(BrowserClient);
};

class FaviconDownloadCallback : public CefDownloadImageCallback {
public:
    explicit FaviconDownloadCallback(CefRefPtr<BrowserClient> client) : client(client) {}

    virtual void OnDownloadImageFinished(const CefString& image_url, int http_status_code, CefRefPtr<CefImage> image) override;

private:
    CefRefPtr<BrowserClient> client;

IMPLEMENT_REFCOUNTING(FaviconDownloadCallback);
DISALLOW_COPY_AND_ASSIGN(FaviconDownloadCallback);
};

class MyWindowDelegate : public CefWindowDelegate {

public:
    struct Options {
        int x = 0, y = 0;
        int w, h;
        bool centerScreen = false;
        bool visible = true;
        bool modal = false;
        Window modalParent;
        std::string title;
    };

    MyWindowDelegate(CefRefPtr<CefBrowserView> browserView, Options options) : browserView(browserView), options(options) {}

    virtual void OnWindowCreated(CefRefPtr<CefWindow> window) override;

    virtual void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
        browserView = NULL;
    }

private:
    CefRefPtr<CefBrowserView> browserView;
    Options options;

IMPLEMENT_REFCOUNTING(MyWindowDelegate);
DISALLOW_COPY_AND_ASSIGN(MyWindowDelegate);
};