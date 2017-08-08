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
    }
    return false;
}

void InitialSetupRenderHandler::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                                 CefRefPtr<CefV8Context> context) {
    printf("InitialSetupRenderHandler::OnContextCreated\n");

    if (!externalInterfaceHandler)
        externalInterfaceHandler = new InitialSetupV8Handler(*this);

    CefRefPtr<CefV8Value> global = context->GetGlobal();
    CefRefPtr<CefV8Value> object = CefV8Value::CreateObject(nullptr, nullptr);
    object->SetValue("startGoogleLogin", CefV8Value::CreateFunction("startGoogleLogin", externalInterfaceHandler), V8_PROPERTY_ATTRIBUTE_NONE);
    global->SetValue("setup", object, V8_PROPERTY_ATTRIBUTE_NONE);
}

bool InitialSetupV8Handler::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& args,
                                    CefRefPtr<CefV8Value>& retval, CefString& exception) {
    if (name == "startGoogleLogin") {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("StartGoogleLogin");
        handler.GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
        return true;
    }
    return false;
}