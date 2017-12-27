#include <X11/Xlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "include/views/cef_browser_view.h"
#include "gamepad_mapper_browser.h"
#include "gamepad.h"
#include "path_helper.h"

AsyncResult<bool> GamepadMapperBrowserClient::resultState;
std::string const GamepadMapperRenderHandler::Name = "GamepadMapperRenderHandler";

void GamepadMapperBrowserClient::OpenBrowser() {
    printf("GamepadMapperBrowserClient::OpenBrowser\n");

    BrowserApp::RunWithContext([] {
        CefRefPtr<GamepadMapperBrowserClient> client = new GamepadMapperBrowserClient();

        MyWindowDelegate::Options options;
        options.w = 720;
        options.h = 576;
        options.centerScreen = true;

        CefBrowserSettings browserSettings;
        browserSettings.web_security = STATE_DISABLED;
        CefRefPtr<CefBrowserView> view = CefBrowserView::CreateBrowserView(
                client, "file://" + PathHelper::findDataFile("src/initial_setup_resources/gamepad-mapper/index.html"), browserSettings, NULL, NULL);
        client->SetPrimaryWindow(CefWindow::CreateTopLevelWindow(new MyWindowDelegate(view, options)));
    });

    resultState.Clear();
    resultState.Get();
}

GamepadMapperBrowserClient::GamepadMapperBrowserClient() {
    SetRenderHandler<GamepadMapperRenderHandler>();

    LinuxGamepadManager::instance.setRawButtonCallback(std::bind(&GamepadMapperBrowserClient::CallbackButtonPressed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

GamepadMapperBrowserClient::~GamepadMapperBrowserClient() {
    LinuxGamepadManager::instance.setRawButtonCallback(nullptr);
}

bool GamepadMapperBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "Finish") {
        CloseAllBrowsers(true);
        return true;
    }
    return false;
}

void GamepadMapperBrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    BrowserClient::OnBeforeClose(browser);
    if (browserList.empty())
        resultState.Set(true);
}

void GamepadMapperBrowserClient::CallbackButtonPressed(int gamepadId, int button, bool pressed) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CallbackButtonPressed");
    msg->GetArgumentList()->SetInt(0, gamepadId);
    msg->GetArgumentList()->SetInt(1, button);
    msg->GetArgumentList()->SetBool(2, pressed);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
}

void GamepadMapperRenderHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefV8Context> context) {
    printf("InitialSetupRenderHandler::OnContextCreated\n");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new GamepadMapperV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("setGamepadButtonCallback", CefV8Value::CreateFunction("setGamepadButtonCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("gamepadMapper", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool GamepadMapperRenderHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "CallbackButtonPressed") {
        externalInterfaceHandler->CallGamepadButtonCallback(message->GetArgumentList()->GetInt(0),
                                                            message->GetArgumentList()->GetInt(1),
                                                            message->GetArgumentList()->GetBool(2));
        return true;
    }
    return false;
}

void GamepadMapperV8Handler::CallGamepadButtonCallback(int gamepadId, int button, bool pressed) {
    auto& cb = gamepadButtonCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {CefV8Value::CreateInt(gamepadId), CefV8Value::CreateInt(button),
                                         CefV8Value::CreateBool(pressed)});
    cb.first->Exit();
}

bool GamepadMapperV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "setGamepadButtonCallback" && args.size() == 1 && args[0]->IsFunction()) {
        gamepadButtonCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "finish" && args.size() == 1 && args[0]->IsBool()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("Finish");
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    return false;
}