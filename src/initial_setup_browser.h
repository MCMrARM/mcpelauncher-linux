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

class InitialSetupV8Handler;

class InitialSetupBrowserClient : public BrowserClient {

private:
    static AsyncResult<bool> resultState;

public:
    static bool OpenBrowser();

    InitialSetupBrowserClient();

    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                          CefRefPtr<CefProcessMessage> message) override;
};

class InitialSetupRenderHandler : public MyRenderProcessHandler {

private:
    CefRefPtr<InitialSetupV8Handler> externalInterfaceHandler;

public:
    InitialSetupRenderHandler(CefRefPtr<CefBrowser> browser) : MyRenderProcessHandler(browser) {}

    static const std::string Name;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) override;

};

class InitialSetupV8Handler : public CefV8Handler {

private:
    InitialSetupRenderHandler& handler;

public:
    InitialSetupV8Handler(InitialSetupRenderHandler& handler) : handler(handler) {}

    virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                         CefRefPtr<CefV8Value>& retval, CefString& exception) override;

private:
IMPLEMENT_REFCOUNTING(InitialSetupV8Handler);
};