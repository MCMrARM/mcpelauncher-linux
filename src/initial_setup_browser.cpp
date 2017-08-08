#include <X11/Xlib.h>
#include "initial_setup_browser.h"
#include "common.h"
#include "google_play_helper.h"

AsyncResult<bool> InitialSetupBrowserClient::resultState;
std::string const InitialSetupRenderHandler::Name = "InitialSetupRenderHandler";

bool InitialSetupBrowserClient::OpenBrowser() {
    printf("InitialSetupBrowserClient::OpenBrowser\n");

    BrowserApp::RunWithContext([] {
        CefRefPtr<InitialSetupBrowserClient> client = new InitialSetupBrowserClient();

        CefWindowInfo window;
        window.width = 720;
        window.height = 576;
        CefBrowserSettings browserSettings;
        CefBrowserHost::CreateBrowser(window, client, "file://" + getCWD() + "/src/initial_setup_resources/index.html", browserSettings, NULL);
    });

    resultState.Clear();
    return resultState.Get();
}

InitialSetupBrowserClient::InitialSetupBrowserClient() {
    SetRenderHandler<InitialSetupRenderHandler>();
}

void InitialSetupBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    BrowserClient::OnBeforeClose(browser);
    if (browserList.empty())
        resultState.Set(false);
}

bool InitialSetupBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "StartGoogleLogin") {
        printf("StartGoogleLogin\n");

        Display* display = cef_get_xdisplay();
        Window window = browser->GetHost()->GetWindowHandle();

        XWindowAttributes attrs;
        XGetWindowAttributes(display, window, &attrs);

        Window rootWindow = XDefaultRootWindow(display);
        int x, y;
        Window child;
        XTranslateCoordinates(display, window, rootWindow, 0, 0, &x, &y, &child);

        CefWindowInfo windowInfo;
        windowInfo.width = 480;
        windowInfo.height = 640;
        windowInfo.x = x + attrs.width / 2 - windowInfo.width / 2;
        windowInfo.y = y + attrs.height / 2 - windowInfo.height / 2;
        GooglePlayHelper::singleton.handleLoginAndApkDownload(this, windowInfo);
        return true;
    } else if (message->GetName() == "SetAskResult") {
        bool result = message->GetArgumentList()->GetBool(0);
        askResult.Set(result);
        return true;
    } else if (message->GetName() == "SetAskTosResult") {
        bool accepted = message->GetArgumentList()->GetBool(0);
        bool marketing = message->GetArgumentList()->GetBool(1);
        askTosResult.Set(accepted ? (marketing ? AskTosResult::ACCEPTED_MARKETING : AskTosResult::ACCEPTED) : AskTosResult::DECLINED);
        return true;
    }
    return false;
}

AsyncResult<bool>& InitialSetupBrowserClient::ShowMessage(std::string const& title, std::string const& text) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("ShowMessage");
    CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
    msgArgs->SetString(0, text);
    msgArgs->SetString(1, title);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
    askResult.Clear();
    return askResult;
}

AsyncResult<bool>& InitialSetupBrowserClient::AskYesNo(std::string const& title, std::string const& text) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AskYesNo");
    CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
    msgArgs->SetString(0, text);
    msgArgs->SetString(1, title);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
    askResult.Clear();
    return askResult;
}

AsyncResult<InitialSetupBrowserClient::AskTosResult>& InitialSetupBrowserClient::AskAcceptTos(std::string const& tos) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("AskAcceptTos");
    CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
    msgArgs->SetString(0, tos);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
    askTosResult.Clear();
    return askTosResult;
}

void InitialSetupBrowserClient::NotifyDownloadStatus(bool downloading, long long downloaded, long long downloadTotal) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("NotifyDownloadStatus");
    CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
    msgArgs->SetBool(0, downloading);
    msgArgs->SetDouble(1, (double) downloaded);
    msgArgs->SetDouble(2, (double) downloadTotal);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
}

void InitialSetupBrowserClient::NotifyApkSetupResult(bool success) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("NotifyApkSetupResult");
    msg->GetArgumentList()->SetBool(0, success);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
}

void InitialSetupRenderHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefV8Context> context) {
    printf("InitialSetupRenderHandler::OnContextCreated\n");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new InitialSetupV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("startGoogleLogin", CefV8Value::CreateFunction("startGoogleLogin", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setMessageHandler", CefV8Value::CreateFunction("setMessageHandler", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setAskResult", CefV8Value::CreateFunction("setAskResult", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setAskTosResult", CefV8Value::CreateFunction("setAskTosResult", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setDownloadStatusCallback", CefV8Value::CreateFunction("setDownloadStatusCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setApkSetupCallback", CefV8Value::CreateFunction("setApkSetupCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("setup", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool InitialSetupRenderHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "ShowMessage") {
        CefRefPtr<CefV8Value> text = CefV8Value::CreateString(message->GetArgumentList()->GetString(0));
        CefRefPtr<CefV8Value> title = CefV8Value::CreateString(message->GetArgumentList()->GetString(1));
        externalInterfaceHandler->CallMessageCallback({CefV8Value::CreateString("message"), text, title});
        return true;
    } else if (message->GetName() == "AskYesNo") {
        CefRefPtr<CefV8Value> text = CefV8Value::CreateString(message->GetArgumentList()->GetString(0));
        CefRefPtr<CefV8Value> title = CefV8Value::CreateString(message->GetArgumentList()->GetString(1));
        externalInterfaceHandler->CallMessageCallback({CefV8Value::CreateString("yesno"), text, title});
        return true;
    } else if (message->GetName() == "AskAcceptTos") {
        CefRefPtr<CefV8Value> text = CefV8Value::CreateString(message->GetArgumentList()->GetString(0));
        externalInterfaceHandler->CallMessageCallback({CefV8Value::CreateString("tos"), text});
        return true;
    } else if (message->GetName() == "NotifyDownloadStatus") {
        auto args = message->GetArgumentList();
        externalInterfaceHandler->NotifyDownloadStatus(args->GetBool(0), args->GetDouble(1), args->GetDouble(2));
        return true;
    } else if (message->GetName() == "NotifyApkSetupResult") {
        externalInterfaceHandler->NotifyApkSetupResult(message->GetArgumentList()->GetBool(0));
        return true;
    }
    return false;
}

void InitialSetupV8Handler::CallMessageCallback(const CefV8ValueList& arguments) {
    auto& cb = messageCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, arguments);
    cb.first->Exit();
}

void InitialSetupV8Handler::NotifyDownloadStatus(bool downloading, double downloaded, double downloadTotal) {
    auto& cb = downloadStatusCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {CefV8Value::CreateBool(downloading), CefV8Value::CreateDouble(downloaded),
                                         CefV8Value::CreateDouble(downloadTotal)});
    cb.first->Exit();
}

void InitialSetupV8Handler::NotifyApkSetupResult(bool success) {
    auto& cb = apkSetupResultCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {CefV8Value::CreateBool(success)});
    cb.first->Exit();
}

bool InitialSetupV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "startGoogleLogin") {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("StartGoogleLogin");
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    } else if (name == "setMessageHandler" && args.size() == 1 && args[0]->IsFunction()) {
        messageCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setDownloadStatusCallback" && args.size() == 1 && args[0]->IsFunction()) {
        downloadStatusCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setApkSetupCallback" && args.size() == 1 && args[0]->IsFunction()) {
        apkSetupResultCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setAskResult" && args.size() == 1 && args[0]->IsBool()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetAskResult");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetBool(0, args[0]->GetBoolValue());
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    } else if (name == "setAskTosResult" && args.size() >= 1 && args[0]->IsBool()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetAskTosResult");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetBool(0, args[0]->GetBoolValue());
        if (args.size() >= 2 && args[1]->IsBool())
            msgArgs->SetBool(1, args[1]->GetBoolValue());
        else
            msgArgs->SetBool(1, false);
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    return false;
}