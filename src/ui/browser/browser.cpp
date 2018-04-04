#include "browser.h"

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <sys/stat.h>
#include "../../common/path_helper.h"
#include "../../common/log.h"

#include "include/base/cef_bind.h"
#include "include/wrapper/cef_closure_task.h"

CefMainArgs BrowserApp::cefMainArgs;
CefRefPtr<BrowserApp> BrowserApp::singleton (new BrowserApp());
std::unordered_map<std::string, std::function<std::shared_ptr<MyRenderProcessHandler> (CefRefPtr<CefBrowser> browser)>> BrowserApp::knownRenderHandlers;

static void RunFunction(std::function<void()> func) {
    func();
}

void BrowserApp::DoCefThread() {
    CefSettings settings;
    settings.no_sandbox = true;
    CefString(&settings.resources_dir_path) = PathHelper::findDataFile("libs/cef/res");
    CefString(&settings.locales_dir_path) = PathHelper::findDataFile("libs/cef/res/locales/");
    CefInitialize(cefMainArgs, settings, singleton.get(), nullptr);
    CefRunMessageLoop();
    CefShutdown();
}

void BrowserApp::RunWithContext(std::function<void()> contextCallback) {
    if (!singleton->cefThread.joinable()) {
        singleton->contextCallback = contextCallback;
        singleton->cefThread = std::thread(DoCefThread);
    } else if (!CefCurrentlyOn(TID_UI)) {
        RunOnUI(contextCallback);
    } else {
        contextCallback();
    }
}

void BrowserApp::RunOnUI(std::function<void()> function) {
    CefPostTask(TID_UI, base::Bind(&RunFunction, function));
}

void BrowserApp::Shutdown() {
    if (singleton->cefThread.joinable()) {
        CefPostTask(TID_UI, base::Bind(CefQuitMessageLoop));

        singleton->cefThread.join();
    }
}

BrowserApp::BrowserApp() {

}

void BrowserApp::SetRenderHandler(int browser, std::string const& handler) {
    if (handler.empty())
        browserRenderHandlers[browser] = std::shared_ptr<MyRenderProcessHandler>(new MyRenderProcessHandler(renderProcessBrowsers.at(browser)));
    else
        browserRenderHandlers[browser] = knownRenderHandlers.at(handler)(renderProcessBrowsers.at(browser));
}

void BrowserApp::OnContextInitialized() {
    contextCallback();
}


bool BrowserApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process,
                                                      CefRefPtr<CefProcessMessage> message) {
    if (browserRenderHandlers.count(browser->GetIdentifier()) > 0 && browserRenderHandlers.at(browser->GetIdentifier())->OnProcessMessageReceived(browser, source_process, message))
        return true;
    if (message->GetName() == "SetRenderHandler") {
        CefRefPtr<CefListValue> args = message->GetArgumentList();
        BrowserApp::singleton->SetRenderHandler(args->GetInt(0), args->GetString(1));
        return true;
    }
    return false;
}


void BrowserApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
    Log::trace("BrowserApp", "OnBrowserCreated %i", browser->GetIdentifier());
    renderProcessBrowsers[browser->GetIdentifier()] = browser;
}

void BrowserApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {
    Log::trace("BrowserApp", "OnBrowserDestroyed %i", browser->GetIdentifier());
    browserRenderHandlers.erase(browser->GetIdentifier());
    renderProcessBrowsers.erase(browser->GetIdentifier());
}

void BrowserApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefRefPtr<CefV8Context> context) {
    browserRenderHandlers.at(browser->GetIdentifier())->OnContextCreated(browser, frame, context);
}


void BrowserClient::OnAfterCreated(CefRefPtr<CefBrowser> browser) {
    browserList.push_back(browser);

    CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SetRenderHandler");
    CefRefPtr<CefListValue> args = msg->GetArgumentList();
    args->SetInt(0, browser->GetIdentifier());
    args->SetString(1, renderHandlerId);
    browser->SendProcessMessage(PID_RENDERER, msg);
}

void BrowserClient::OnBeforeClose(CefRefPtr<CefBrowser> browser) {
    for (auto bit = browserList.begin(); bit != browserList.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browserList.erase(bit);
            break;
        }
    }
}

void BrowserClient::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                                const CefString& errorText, const CefString& failedUrl) {
    if (errorCode == ERR_ABORTED)
        return;

    std::stringstream ss;
    ss << "<html><body><h2>Failed to load URL " << std::string(failedUrl) << "</h2>"
       << "<p>" << std::string(errorText) << " (" << errorCode << ").</p></body></html>";
    frame->LoadString(ss.str(), failedUrl);
}

void BrowserClient::CloseAllBrowsers(bool forceClose) {
    if (!CefCurrentlyOn(TID_UI)) {
        CefPostTask(TID_UI, base::Bind(&BrowserClient::CloseAllBrowsers, this, forceClose));
        return;
    }
    for (auto it = browserList.begin(); it != browserList.end(); ++it)
        (*it)->GetHost()->CloseBrowser(forceClose);
}

void BrowserClient::OnFaviconURLChange(CefRefPtr<CefBrowser> browser, const std::vector<CefString>& icon_urls) {
    CefDisplayHandler::OnFaviconURLChange(browser, icon_urls);
    if (!icon_urls.empty())
        browser->GetHost()->DownloadImage(icon_urls[0], true, 16, false, new FaviconDownloadCallback(this));
}

void BrowserClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) {
    GetPrimaryWindow()->SetTitle(title);
}

bool BrowserClient::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
                                  const CefString& target_frame_name,
                                  CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
                                  const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo,
                                  CefRefPtr<CefClient>& client, CefBrowserSettings& settings,
                                  bool* no_javascript_access) {
    CefRefPtr<BrowserClient> c = new BrowserClient();

    MyWindowDelegate::Options options;
    options.w = 1024;
    options.h = 640;
    options.visible = true;

    CefBrowserSettings browserSettings;
    CefRefPtr<CefBrowserView> view = CefBrowserView::CreateBrowserView(c, target_url, browserSettings, NULL, NULL);
    c->SetPrimaryWindow(CefWindow::CreateTopLevelWindow(new MyWindowDelegate(view, options)));
    return true;
}

void FaviconDownloadCallback::OnDownloadImageFinished(const CefString& image_url, int http_status_code,
                                                      CefRefPtr<CefImage> image) {
    if (image)
        client->GetPrimaryWindow()->SetWindowIcon(image);
}

void MyWindowDelegate::OnWindowCreated(CefRefPtr<CefWindow> window) {
    window->AddChildView(browserView);
    window->SetPosition({options.x, options.y});
    window->SetSize({options.w, options.h});
    window->SetTitle(options.title);
    if (options.modal) {
        Atom wmStateAtom = XInternAtom(cef_get_xdisplay(), "_NET_WM_STATE", False);
        Atom modalAtom = XInternAtom(cef_get_xdisplay(), "_NET_WM_STATE_MODAL", False);
        XChangeProperty(cef_get_xdisplay(), window->GetWindowHandle(), wmStateAtom, XA_ATOM, 32, PropModeAppend, (unsigned char*) &modalAtom, 1);
        XSetTransientForHint(cef_get_xdisplay(), window->GetWindowHandle(), options.modalParent);
    }
    struct stat sb;
    std::string iconPath = PathHelper::getIconPath();
    if (!stat(iconPath.c_str(), &sb)) {
        FILE* file = fopen(iconPath.c_str(), "r");
        char* buf = new char[sb.st_size];
        size_t o = 0, i;
        while ((i = fread(buf, sizeof(char), (size_t) sb.st_size - o, file)) > 0)
            o += i;
        fclose(file);
        CefRefPtr<CefImage> image = CefImage::CreateImage();
        image->AddPNG(1.f, buf, o);
        delete[] buf;
        window->SetWindowIcon(image);
        window->SetWindowAppIcon(image);
    }
    if (options.visible)
        window->Show();
    if (options.centerScreen) {
        XSizeHints sizehints;
        sizehints.win_gravity = CenterGravity;
        sizehints.flags = PWinGravity;
        XSetNormalHints(cef_get_xdisplay(), window->GetWindowHandle(), &sizehints);
    }

    browserView->RequestFocus();
}