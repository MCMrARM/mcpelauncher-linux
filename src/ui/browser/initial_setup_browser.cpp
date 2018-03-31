#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "include/views/cef_browser_view.h"
#include "initial_setup_browser.h"
#include "../../common/common.h"
#include "../../common/log.h"
#include "../../common/extract.h"
#include "../../common/path_helper.h"
#include "../file_picker/file_picker_factory.h"

#ifndef DISABLE_PLAYAPI
#include "google_play_helper.h"
#endif

AsyncResult<bool> InitialSetupBrowserClient::resultState;
std::string const InitialSetupRenderHandler::Name = "InitialSetupRenderHandler";

bool InitialSetupBrowserClient::OpenBrowser() {
    Log::trace("InitialSetupBrowserClient", "OpenBrowser");

    BrowserApp::RunWithContext([] {
        CefRefPtr<InitialSetupBrowserClient> client = new InitialSetupBrowserClient();

        MyWindowDelegate::Options options;
        options.w = 720;
        options.h = 576;
        options.centerScreen = true;

        CefBrowserSettings browserSettings;
        CefRefPtr<CefBrowserView> view = CefBrowserView::CreateBrowserView(
                client, "file://" + PathHelper::findDataFile("src/client/initial_setup_resources/index.html"), browserSettings, NULL, NULL);
        client->SetPrimaryWindow(CefWindow::CreateTopLevelWindow(new MyWindowDelegate(view, options)));
    });

    resultState.Clear();
    return resultState.Get();
}

InitialSetupBrowserClient::InitialSetupBrowserClient() {
    SetRenderHandler<InitialSetupRenderHandler>();
}

void InitialSetupBrowserClient::HandlePickFile(std::string const& title, std::string const& ext) {
    try {
        auto picker = FilePickerFactory::createFilePicker();
        picker->setTitle(title);
        picker->setFileNameFilters({ext});

        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("PickFileResult");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        if (picker->show())
            msgArgs->SetString(0, picker->getPickedFile());
        else
            msgArgs->SetString(1, "");
        GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
    } catch (std::exception& err) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("PickFileResult");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetString(1, err.what());
        GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
        return;
    }
}

void InitialSetupBrowserClient::HandleSetupWithFile(std::string const& file) {
    Log::trace("InitialSetupBrowserClient", "SetupWithFile %s", file.c_str());
    try {
        ExtractHelper::extractApk(file);
        NotifyApkSetupResult(true);
    } catch (std::runtime_error& e) {
        NotifyApkSetupResult(false);
    }
}

void InitialSetupBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    BrowserClient::OnBeforeClose(browser);
    if (browserList.empty())
        resultState.Set(false);
}

bool InitialSetupBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "StartGoogleLogin") {
        Log::trace("InitialSetupBrowserClient", "StartGoogleLogin");

#ifdef DISABLE_PLAYAPI
        ShowMessage("Feature disabled", "Google account login was disabled during compilation.");
        NotifyApkSetupResult(false);
#else
        Display* display = cef_get_xdisplay();
        Window window = browser->GetHost()->GetWindowHandle();

        XWindowAttributes attrs;
        XGetWindowAttributes(display, window, &attrs);

        Window rootWindow = XDefaultRootWindow(display);
        int x, y;
        Window child;
        XTranslateCoordinates(display, window, rootWindow, 0, 0, &x, &y, &child);

        MyWindowDelegate::Options windowInfo;
        windowInfo.w = 480;
        windowInfo.h = 640;
        windowInfo.x = x - attrs.x + attrs.width / 2 - windowInfo.w / 2;
        windowInfo.y = y - attrs.y + attrs.height / 2 - windowInfo.h / 2;
        windowInfo.modal = true;
        windowInfo.modalParent = browser->GetHost()->GetWindowHandle();
        GooglePlayHelper::singleton.handleLoginAndApkDownload(this, windowInfo);
#endif
        return true;
    } else if (message->GetName() == "SetupWithFile") {
        std::string file = message->GetArgumentList()->GetString(0);
        std::thread t([this, file] {
            HandleSetupWithFile(file);
        });
        t.detach();
        return true;
    }  else if (message->GetName() == "SetAskResult") {
        bool result = message->GetArgumentList()->GetBool(0);
        askResult.Set(result);
        return true;
    } else if (message->GetName() == "SetAskTosResult") {
        bool accepted = message->GetArgumentList()->GetBool(0);
        bool marketing = message->GetArgumentList()->GetBool(1);
        askTosResult.Set(accepted ? (marketing ? AskTosResult::ACCEPTED_MARKETING : AskTosResult::ACCEPTED) : AskTosResult::DECLINED);
        return true;
    } else if (message->GetName() == "PickFile") {
        std::string title = message->GetArgumentList()->GetString(0);
        std::string ext = message->GetArgumentList()->GetString(1);
        std::thread t([this, title, ext] {
            HandlePickFile(title, ext);
        });
        t.detach();
        return true;
    } else if (message->GetName() == "Finish") {
        resultState.Set(true);
        CloseAllBrowsers(true);
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
    Log::trace("InitialSetupBrowserClient", "OnContextCreated");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new InitialSetupV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("startGoogleLogin", CefV8Value::CreateFunction("startGoogleLogin", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setupWithFile", CefV8Value::CreateFunction("setupWithFile", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setMessageHandler", CefV8Value::CreateFunction("setMessageHandler", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setAskResult", CefV8Value::CreateFunction("setAskResult", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setAskTosResult", CefV8Value::CreateFunction("setAskTosResult", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setDownloadStatusCallback", CefV8Value::CreateFunction("setDownloadStatusCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setApkSetupCallback", CefV8Value::CreateFunction("setApkSetupCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("pickFile", CefV8Value::CreateFunction("pickFile", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("finish", CefV8Value::CreateFunction("finish", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
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
    } else if (message->GetName() == "PickFileResult") {
        externalInterfaceHandler->CallPickFileCallback(message->GetArgumentList()->GetString(0),
                                                       message->GetArgumentList()->GetString(1));
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

void InitialSetupV8Handler::CallPickFileCallback(std::string const& file, std::string const& error) {
    CefRefPtr<CefV8Value> fileVal = file.empty() ? CefV8Value::CreateNull() : CefV8Value::CreateString(file);
    CefRefPtr<CefV8Value> errorVal = error.empty() ? CefV8Value::CreateNull() : CefV8Value::CreateString(error);
    auto& cb = pickFileCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {fileVal, errorVal});
    cb.first->Exit();
}

bool InitialSetupV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "startGoogleLogin") {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("StartGoogleLogin");
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    } else if (name == "setupWithFile" && args.size() == 1 && args[0]->IsString()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetupWithFile");
        msg->GetArgumentList()->SetString(0, args[0]->GetStringValue());
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
    } else if (name == "pickFile" && args.size() == 3 && args[0]->IsString() && args[1]->IsString() && args[2]->IsFunction()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("PickFile");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetString(0, args[0]->GetStringValue());
        msgArgs->SetString(1, args[1]->GetStringValue());
        pickFileCallback = {CefV8Context::GetCurrentContext(), args[2]};
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    } else if (name == "finish" && args.size() == 1 && args[0]->IsBool()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("Finish");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetBool(0, args[0]->GetBoolValue());
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    return false;
}