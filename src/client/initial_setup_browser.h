#pragma once

#include <list>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "../minecraft/Xbox.h"
#include "../common/browser.h"
#include "../common/async_result_util.h"

class InitialSetupV8Handler;

class InitialSetupBrowserClient : public BrowserClient {

public:
    enum class AskTosResult {
        ACCEPTED, ACCEPTED_MARKETING, DECLINED
    };

private:
    static AsyncResult<bool> resultState;

    AsyncResult<bool> askResult;
    AsyncResult<AskTosResult> askTosResult;

    void HandleSetupWithFile(std::string const& file);

    void HandlePickFile(std::string const& title, std::string const& ext);

public:
    static bool OpenBrowser();

    InitialSetupBrowserClient();

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

    AsyncResult<bool>& ShowMessage(std::string const& title, std::string const& text);

    AsyncResult<bool>& AskYesNo(std::string const& title, std::string const& text);

    AsyncResult<AskTosResult>& AskAcceptTos(std::string const& tos);

    void NotifyDownloadStatus(bool downloading, long long downloaded = 0, long long downloadTotal = 0);

    void NotifyApkSetupResult(bool success);

};

class InitialSetupRenderHandler : public MyRenderProcessHandler {

private:
    CefRefPtr<InitialSetupV8Handler> externalInterfaceHandler;

public:
    InitialSetupRenderHandler(CefRefPtr<CefBrowser> browser) : MyRenderProcessHandler(browser) {}

    static const std::string Name;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;

};

class InitialSetupV8Handler : public CefV8Handler {

private:
    InitialSetupRenderHandler& handler;
    std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>> messageCallback;
    std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>> downloadStatusCallback;
    std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>> apkSetupResultCallback;
    std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>> pickFileCallback;

public:

    void CallMessageCallback(const CefV8ValueList& arguments);
    void NotifyDownloadStatus(bool downloading, double downloaded, double downloadTotal);
    void NotifyApkSetupResult(bool success);
    void CallPickFileCallback(std::string const& file, std::string const& error);

    InitialSetupV8Handler(InitialSetupRenderHandler& handler) : handler(handler) {}

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
IMPLEMENT_REFCOUNTING(InitialSetupV8Handler);
};