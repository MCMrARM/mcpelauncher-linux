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
        printf("%s",PathHelper::findDataFile("src/initial_setup_resources/gamepad-mapper/index.html"));
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
    LinuxGamepadManager::instance.setRawStickCallback(std::bind(&GamepadMapperBrowserClient::CallbackStick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    LinuxGamepadManager::instance.setRawHatCallback(std::bind(&GamepadMapperBrowserClient::CallbackHat, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

GamepadMapperBrowserClient::~GamepadMapperBrowserClient() {
    LinuxGamepadManager::instance.setRawButtonCallback(nullptr);
    LinuxGamepadManager::instance.setRawStickCallback(nullptr);
    LinuxGamepadManager::instance.setRawHatCallback(nullptr);
}

bool GamepadMapperBrowserClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "SetGamepadMapping") {
        int index = message->GetArgumentList()->GetInt(0);
        std::string mapping = message->GetArgumentList()->GetString(1);
        printf("Set Mapping = %s\n", mapping.c_str());
        // TODO: Synchronize this
        LinuxGamepadManager::instance.setGamepadMapping(LinuxGamepadManager::instance.getGamepadTypeId(index), mapping);
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

void GamepadMapperBrowserClient::CallbackStick(int gamepadId, int stick, float value) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CallbackStick");
    msg->GetArgumentList()->SetInt(0, gamepadId);
    msg->GetArgumentList()->SetInt(1, stick);
    msg->GetArgumentList()->SetDouble(2, value);
    GetPrimaryBrowser()->SendProcessMessage(PID_RENDERER, msg);
}

void GamepadMapperBrowserClient::CallbackHat(int gamepadId, int stick, int value) {
    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("CallbackHat");
    msg->GetArgumentList()->SetInt(0, gamepadId);
    msg->GetArgumentList()->SetInt(1, stick);
    msg->GetArgumentList()->SetInt(2, value);
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
    object->SetValue("setGamepadStickCallback", CefV8Value::CreateFunction("setGamepadStickCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setGamepadHatCallback", CefV8Value::CreateFunction("setGamepadHatCallback", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    object->SetValue("setGamepadMapping", CefV8Value::CreateFunction("setGamepadMapping", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("gamepadMapper", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool GamepadMapperRenderHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                         CefRefPtr<CefProcessMessage> message) {
    if (message->GetName() == "CallbackButtonPressed") {
        externalInterfaceHandler->CallGamepadButtonCallback(message->GetArgumentList()->GetInt(0),
                                                            message->GetArgumentList()->GetInt(1),
                                                            message->GetArgumentList()->GetBool(2));
        return true;
    } else if (message->GetName() == "CallbackStick") {
        externalInterfaceHandler->CallGamepadStickCallback(message->GetArgumentList()->GetInt(0),
                                                           message->GetArgumentList()->GetInt(1),
                                                           message->GetArgumentList()->GetDouble(2));
        return true;
    } else if (message->GetName() == "CallbackHat") {
        externalInterfaceHandler->CallGamepadHatCallback(message->GetArgumentList()->GetInt(0),
                                                         message->GetArgumentList()->GetInt(1),
                                                         message->GetArgumentList()->GetInt(2));
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

void GamepadMapperV8Handler::CallGamepadStickCallback(int gamepadId, int stick, double value) {
    auto& cb = gamepadStickCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {CefV8Value::CreateInt(gamepadId), CefV8Value::CreateInt(stick),
                                         CefV8Value::CreateDouble(value)});
    cb.first->Exit();
}

void GamepadMapperV8Handler::CallGamepadHatCallback(int gamepadId, int stick, int value) {
    auto& cb = gamepadHatCallback;
    cb.first->Enter();
    cb.second->ExecuteFunction(nullptr, {CefV8Value::CreateInt(gamepadId), CefV8Value::CreateInt(stick),
                                         CefV8Value::CreateInt(value)});
    cb.first->Exit();
}

bool GamepadMapperV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "setGamepadButtonCallback" && args.size() == 1 && args[0]->IsFunction()) {
        gamepadButtonCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setGamepadStickCallback" && args.size() == 1 && args[0]->IsFunction()) {
        gamepadStickCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setGamepadHatCallback" && args.size() == 1 && args[0]->IsFunction()) {
        gamepadHatCallback = {CefV8Context::GetCurrentContext(), args[0]};
        return true;
    } else if (name == "setGamepadMapping" && args.size() == 2 && args[0]->IsInt() && args[1]->IsString()) {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetGamepadMapping");
        CefRefPtr<CefListValue> msgArgs = msg->GetArgumentList();
        msgArgs->SetInt(0, args[0]->GetIntValue());
        msgArgs->SetString(1, args[1]->GetStringValue());
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    return false;
}
