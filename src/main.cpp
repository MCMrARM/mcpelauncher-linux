#include <iostream>
#include <dlfcn.h>
#include <stdarg.h>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <codecvt>
#include <locale>
#include <dirent.h>
#include <subhook.h>
#include "gles_symbols.h"
#include "android_symbols.h"
#include "egl_symbols.h"
#include "fmod_symbols.h"
#include "../mcpe/gl.h"
#include "../mcpe/AppPlatform.h"
#include "../mcpe/MinecraftClient.h"
#include "LinuxAppPlatform.h"
#include "LinuxStore.h"
#include "../mcpe/Mouse.h"
#include "../mcpe/Keyboard.h"
#include "../mcpe/Options.h"

extern "C" {

#include <eglut.h>
#include "../hybris/include/hybris/dlfcn.h"
#include "../hybris/include/hybris/hook.h"
#include "../hybris/src/jb/linker.h"

void __android_log_vprint(int prio, const char *tag,  const char *fmt, va_list args) {
    std::cout << "[" << tag << "] ";
    vprintf(fmt, args);
    std::cout << std::endl;
}
void __android_log_print(int prio, const char *tag,  const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(prio, tag, fmt, args);
    va_end(args);
}
void __android_log_write(int prio, const char *tag,  const char *fmt, const char *text) {
    std::cout << "[" << tag << "] " << text << std::endl;
}

}

std::string getCWD() {
    char _cwd[MAXPATHLEN];
    getcwd(_cwd, MAXPATHLEN);
    return std::string(_cwd) + "/";
}

bool loadLibrary(std::string path) {
    void* handle = hybris_dlopen((getCWD() + "libs/" + path).c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "failed to load library " << path << ": " << hybris_dlerror() << "\n";
        return false;
    }
    return true;
}

void* loadLibraryOS(std::string path, const char** symbols) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "failed to load library " << path << ": " << dlerror() << "\n";
        return nullptr;
    }
    std::cout << "oslib: " << path << ": " << (int) handle << "\n";
    int i = 0;
    while (true) {
        const char* sym = symbols[i];
        if (sym == nullptr)
            break;
        void* ptr = dlsym(handle, sym);
        hybris_hook(sym, ptr);
        i++;
    }
    return handle;
}

void androidStub() {
    std::cout << "warn: android call\n";
}

void eglStub() {
    std::cout << "warn: egl call\n";
}

void stubSymbols(const char** symbols, void* stubfunc) {
    int i = 0;
    while (true) {
        const char* sym = symbols[i];
        if (sym == nullptr)
            break;
        hybris_hook(sym, stubfunc);
        i++;
    }
}

std::unique_ptr<LinuxStore> createStoreHookFunc(const std::string& idk, StoreListener& listener) {
    std::cout << "creating fake store <" << idk << ">\n";
    return std::unique_ptr<LinuxStore>(new LinuxStore());
}

class HTTPRequest;

class LinuxHttpRequestInternal {
public:
    void* vtable;
    int filler1;
    HTTPRequest* request;

    void destroy() {
        std::cout << "destroying http request\n";
    }
};
void** linuxHttpRequestInternalVtable;

void constructLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal, HTTPRequest* request) {
    requestInternal->vtable = linuxHttpRequestInternalVtable;
    requestInternal->request = request;
}

void sendLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal) {
    std::cout << "send http request\n";
    // TODO: Implement it
}

void abortLinuxHttpRequestInternal(LinuxHttpRequestInternal* requestInternal) {
    std::cout << "abort http request\n";
    // TODO: Implement it
}


static MinecraftClient* client;

int winId = 0;
bool moveMouseToCenter = false;

static void minecraft_idle() {
    if (client->wantToQuit()) {
        eglutDestroyWindow(winId);
        eglutFini();
        return;
    }
    int cx = eglutGetWindowWidth() / 2;
    int cy = eglutGetWindowHeight() / 2;
    if (moveMouseToCenter) {
        eglutWarpMousePointer(cx, cy);
        moveMouseToCenter = false;
    }
    eglutPostRedisplay();
}
static void minecraft_draw() {
    client->update();
}
float pixelSize = 2.f;
static void minecraft_reshape(int w, int h) {
    client->setRenderingSize(w, h);
    client->setUISizeAndScale(w, h, pixelSize);
}
static void minecraft_mouse(int x, int y) {
    if (LinuxAppPlatform::mousePointerHidden) {
        int cx = eglutGetWindowWidth() / 2;
        int cy = eglutGetWindowHeight() / 2;
        if (x != cy || y != cy) {
            Mouse::feed(0, 0, x, y, x - cx, y - cy);
            moveMouseToCenter = true;
        }
    } else {
        Mouse::feed(0, 0, x, y, 0, 0);
    }
}
static void minecraft_mouse_button(int x, int y, int btn, int action) {
    int mcBtn = (btn == 1 ? 1 : (btn == 2 ? 3 : (btn == 3 ? 2 : (btn == 5 ? 4 : btn))));
    Mouse::feed((char) mcBtn, (char) (action == EGLUT_MOUSE_PRESS ? (btn == 5 ? -120 : (btn == 4 ? 120 : 1)) : 0), x, y, 0, 0);
}

int getKeyMinecraft(int keyCode) {
    if (keyCode == 65505)
        return 16;
    if (keyCode >= 97 && keyCode <= 122)
        return (keyCode + 65 - 97);
    if (keyCode >= 65470 && keyCode <= 65481)
        return (keyCode + 112 - 65470);

    return keyCode;
}
static void minecraft_keyboard(char str[5], int action) {
    if (strcmp(str, "\t") == 0)
        return;
    if (action == EGLUT_KEY_PRESS || action == EGLUT_KEY_REPEAT) {
        if (str[0] == 13) {
            str[0] = 10;
            str[1] = 0;
        }
        std::stringstream ss;
        ss << str;
        Keyboard::Keyboard_feedText(ss.str(), false, 0);
    }
}
static void minecraft_keyboard_special(int key, int action) {
    if (key == 65480) {
        if (action == EGLUT_KEY_PRESS) {
            client->getOptions()->setFullscreen(!client->getOptions()->getFullscreen());
        }
        return;
    }
    int mKey = getKeyMinecraft(key);
    if (action == EGLUT_KEY_PRESS) {
        Keyboard::inputs->push_back({1, mKey});
        Keyboard::states[mKey] = 1;
    } else if (action == EGLUT_KEY_RELEASE) {
        Keyboard::inputs->push_back({0, mKey});
        Keyboard::states[mKey] = 0;
    }
}
static void minecraft_close() {
    client->quit();
}

void patchCallInstruction(void* patchOff, void* func, bool jump) {
    unsigned char* data = (unsigned char*) patchOff;
    printf("original: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);
    data[0] = (unsigned char) (jump ? 0xe9 : 0xe8);
    int ptr = ((int) func) - (int) patchOff - 5;
    memcpy(&data[1], &ptr, sizeof(int));
    printf("post patch: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);
}

void unhookFunction(void* hook) {
    SubHook* shook = (SubHook*) hook;
    shook->Remove();
    delete shook;
}

void* hookFunction(void* symbol, void* hook, void** original) {
    SubHook* ret = new SubHook();
    ret->Install(symbol, hook);
    *original = ret->GetTrampoline();
    return ret;
}

void* loadMod(std::string path) {
    void* handle = hybris_dlopen((getCWD() + "mods/" + path).c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "failed to load mod: " << path << "\n";
        return nullptr;
    }

    void (*initFunc)();
    initFunc = (void (*)()) hybris_dlsym(handle, "mod_init");
    if (((void*) initFunc) == nullptr) {
        std::cout << "warn: mod " << path << " doesn't have a init function\n";
        return handle;
    }
    initFunc();

    return handle;
}

/*
 * check_os_64bit
 * 
 * from http://stackoverflow.com/a/6200504
 *
 * Returns integer:
 *   1 = it is a 64-bit OS
 *   0 = it is NOT a 64-bit OS (probably 32-bit)
 *   < 0 = failure
 *     -1 = popen failed
 *     -2 = fgets failed
 *
 * **WARNING**
 * Be CAREFUL! Just testing for a boolean return may not cut it
 * with this (trivial) implementation! (Think of when it fails,
 * returning -ve; this could be seen as non-zero & therefore true!)
 * Suggestions?
 */
static int check_os_64bit(void)
{
    FILE *fp=NULL;
    char cb64[3];

    fp = popen ("getconf LONG_BIT", "r");
    if (!fp)
       return -1;

    if (!fgets(cb64, 3, fp))
        return -2;

    if (!strncmp (cb64, "64", 3)) {
        return 1;
    }
    else {
        return 0;
    }
}

std::string getOSLibraryPath(std::string libName) {
    std::string p = std::string("/usr/lib/i386-linux-gnu/") + libName;
    if (access(p.c_str(), F_OK) != -1) {
        return p;
    }
    p = std::string("/usr/lib32/") + libName;
    if (access(p.c_str(), F_OK) != -1) {
        return p;
    }
    p = std::string("/lib32/") + libName;
    if (access(p.c_str(), F_OK) != -1) {
        return p;
    }
    // if this is a 32-bit machine, we need to check normal lib dirs
    if (check_os_64bit() == 0) {
        p = std::string("/usr/lib/") + libName;
        if (access(p.c_str(), F_OK) != -1) {
            return p;
        }
    }

    std::cout << "could not find os library: " << libName << "\n";
    abort();
}

#include <execinfo.h>
#include <cxxabi.h>
bool hasCrashed = false;
void handleSignal(int signal) {
    printf("Signal %i received\n", signal);
    if (hasCrashed)
        return;
    hasCrashed = true;
    printf("Getting stacktrace...\n");
    void *array[25];
    int count = backtrace(array, 25);
    char **symbols = backtrace_symbols(array, count);
    char *nameBuf = (char*) malloc(256);
    size_t nameBufLen = 256;
    printf("Backtrace elements: %i\n", count);
    for (int i = 0; i < count; i++) {
        if (symbols[i] == nullptr) {
            printf("#%i unk [0x%04x]\n", i, (int)array[i]);
            continue;
        }
        if (symbols[i][0] == '[') { // unknown symbol
            Dl_info symInfo;
            if (hybris_dladdr(array[i], &symInfo)) {
                int status = 0;
                nameBuf = abi::__cxa_demangle(symInfo.dli_sname, nameBuf, &nameBufLen, &status);
                printf("#%i HYBRIS %s+%i in %s+0x%04x [0x%04x]\n", i, nameBuf, (unsigned int) array[i] - (unsigned int) symInfo.dli_saddr, symInfo.dli_fname, (unsigned int) array[i] - (unsigned int) symInfo.dli_fbase, (int)array[i]);
                continue;
            }
        }
        printf("#%i %s\n", i, symbols[i]);
    }
    free(symbols);
    abort();
}

void detachFromJavaStub() {
    std::cout << "detach from java\n";
}
void* getJVMEnvStub() {
    std::cout << "getjvmenv\n";
    return nullptr;
}
bool verifyCertChainStub() {
    std::cout << "verifycertchain\n";
    return true;
}
struct xboxSingleton {
    char filler[8];
};
xboxSingleton xboxGetAppConfigSingleton() {
    std::cout << "xbox get app config singleton\n";
    return xboxSingleton();
}
void xboxConfigSetSandboxStub() {
    std::cout << "xbox config: set sandbox (stub)\n";
}
void patchNotesModelStub() {
    std::cout << "fetch patch notes\n";
}


extern "C"
void pshufb(char* dest, char* src) {
    char new_dest[16];
    for (int i = 0; i < 16; i++)
        new_dest[i] = (src[i] & 0x80) ? 0 : dest[src[i] & 15];
    memcpy(dest, new_dest, 16);
}
extern "C"
void pshufb_xmm4_xmm0();
#include <functional>
#include <sys/mman.h>
#include <EGL/egl.h>

using namespace std;
int main(int argc, char *argv[]) {
    bool enableStackTracePrinting = true;
    bool workaroundAMD = false;

    int windowWidth = 720;
    int windowHeight = 480;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--scale") == 0) {
            i++;
            pixelSize = std::stof(argv[i]);
        } else if (strcmp(argv[i], "-sw") == 0 || strcmp(argv[i], "--width") == 0) {
            i++;
            windowWidth = std::stoi(argv[i]);
        } else if (strcmp(argv[i], "-sh") == 0 || strcmp(argv[i], "--height") == 0) {
            i++;
            windowHeight = std::stoi(argv[i]);
        } else if (strcmp(argv[i], "-ns") == 0 || strcmp(argv[i], "--no-stacktrace") == 0) {
            enableStackTracePrinting = false;
        } else if (strcmp(argv[i], "--pocket-guis") == 0) {
            enablePocketGuis = true;
        } else if (strcmp(argv[i], "--amd-fix") == 0) {
            std::cout << "--amd-fix: Enabling AMD Workaround.\n";
            workaroundAMD = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            std::cout << "Help\n";
            std::cout << "--help               Shows this help information\n";
            std::cout << "--scale <scale>      Sets the pixel scale\n";
            std::cout << "--width <width>      Sets the window width\n";
            std::cout << "--height <height>    Sets the window height\n";
            std::cout << "--pocket-guis        Switches to Pocket Edition GUIs\n";
            std::cout << "--no-stacktrace      Disables stack trace printing\n";
            std::cout << "--amd-workaround     Fixes crashes on pre-i686 and AMD CPUs\n\n";
            std::cout << "EGL Options\n";
            std::cout << "-display <display>  Sets the display\n";
            std::cout << "-info               Shows info about the display\n\n";
            std::cout << "MCPE arguments:\n";
            std::cout << "edu <true|false>\n";
            std::cout << "mcworld <world>\n";
            return 0;
        }
    }

    if (enableStackTracePrinting) {
        struct sigaction act;
        act.sa_handler = handleSignal;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGSEGV, &act, 0);
        sigaction(SIGABRT, &act, 0);
    }

    std::cout << "loading native libraries\n";
    void* glesLib = loadLibraryOS(getOSLibraryPath("libGLESv2.so"), gles_symbols);
    void* fmodLib = loadLibraryOS((getCWD() + "libs/native/libfmod.so.8.2").c_str(), fmod_symbols);
    if (glesLib == nullptr || fmodLib == nullptr)
        return -1;
    std::cout << "loading hybris libraries\n";
    stubSymbols(android_symbols, (void*) androidStub);
    stubSymbols(egl_symbols, (void*) eglStub);
    hybris_hook("eglGetProcAddress", (void*) eglGetProcAddress);
    hybris_hook("mcpelauncher_hook", (void*) hookFunction);
    hybris_hook("mcpelauncher_unhook", (void*) unhookFunction);
    hybris_hook("__android_log_print", (void*) __android_log_print);
    hybris_hook("__android_log_vprint", (void*) __android_log_vprint);
    hybris_hook("__android_log_write", (void*) __android_log_write);
    if (!loadLibrary("libc.so") || !loadLibrary("libstdc++.so") || !loadLibrary("libm.so") || !loadLibrary("libz.so"))
        return -1;
    // load stub libraries
    if (!loadLibrary("libandroid.so") || !loadLibrary("liblog.so") || !loadLibrary("libEGL.so") || !loadLibrary("libGLESv2.so") || !loadLibrary("libOpenSLES.so") || !loadLibrary("libfmod.so") || !loadLibrary("libGLESv1_CM.so"))
        return -1;
    if (!loadLibrary("libmcpelauncher_mod.so"))
        return -1;
    std::cout << "loading MCPE\n";
    void* handle = hybris_dlopen((getCWD() + "libs/libminecraftpe.so").c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        std::cout << "failed to load MCPE: " << hybris_dlerror() << "\n";
        return -1;
    }

    unsigned int libBase = ((soinfo*) handle)->base;
    std::cout << "loaded MCPE (at " << libBase << ")\n";

    DIR *dir;
    struct dirent *ent;
    std::vector<void*> mods;
    if ((dir = opendir ("mods/")) != NULL) {
        std::cout << "loading mods\n";
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            std::string fileName (ent->d_name);
            int len = fileName.length();
            if (len < 4 || fileName[len - 3] != '.' || fileName[len - 2] != 's' || fileName[len - 1] != 'o')
                continue;
            std::cout << "loading: " << fileName << "\n";
            void* mod = loadMod(fileName);
            if (mod != nullptr)
                mods.push_back(mod);
        }
        closedir(dir);
        std::cout << "loaded " << mods.size() << " mods\n";
    }

    std::cout << "apply patches\n";

    /*
    unsigned int patchOff = (unsigned int) hybris_dlsym(handle, "_ZN12StoreFactory11createStoreER13StoreListener") + 66;
    patchCallInstruction((void*) patchOff, (void*) &createStoreHookFunc, false);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN11HTTPRequestC2ERKSs") + 154;
    patchCallInstruction((void*) patchOff, (void*) &constructLinuxHttpRequestInternal, false);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN11HTTPRequest4sendEv") + 26;
    patchCallInstruction((void*) patchOff, (void*) &sendLinuxHttpRequestInternal, false);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN11HTTPRequest5abortEv") + 26;
    patchCallInstruction((void*) patchOff, (void*) &abortLinuxHttpRequestInternal, false);
     */
    unsigned int patchOff = (unsigned int) hybris_dlsym(handle, "_ZN12AndroidStore21createGooglePlayStoreERKSsR13StoreListener");
    patchCallInstruction((void*) patchOff, (void*) &createStoreHookFunc, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroidC2ER11HTTPRequest");
    patchCallInstruction((void*) patchOff, (void*) &constructLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroid4sendEv");
    patchCallInstruction((void*) patchOff, (void*) &sendLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN26HTTPRequestInternalAndroid5abortEv");
    patchCallInstruction((void*) patchOff, (void*) &abortLinuxHttpRequestInternal, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9crossplat10threadpool16detach_from_javaEPv");
    patchCallInstruction((void*) patchOff, (void*) &detachFromJavaStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9crossplat11get_jvm_envEv");
    patchCallInstruction((void*) patchOff, (void*) &getJVMEnvStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN3web4http6client7details22verify_X509_cert_chainERKSt6vectorISsSaISsEERKSs");
    patchCallInstruction((void*) patchOff, (void*) &verifyCertChainStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services20xbox_live_app_config24get_app_config_singletonEv");
    patchCallInstruction((void*) patchOff, (void*) &xboxGetAppConfigSingleton, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services20xbox_live_app_config11set_sandboxESs");
    patchCallInstruction((void*) patchOff, (void*) &xboxConfigSetSandboxStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN4xbox8services20xbox_live_app_config29set_title_telemetry_device_idERKSs");
    patchCallInstruction((void*) patchOff, (void*) &xboxConfigSetSandboxStub, true);

    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN15PatchNotesModel17preloadPatchNotesEv");
    patchCallInstruction((void*) patchOff, (void*) &patchNotesModelStub, true);

    linuxHttpRequestInternalVtable = (void**) ::operator new(8);
    linuxHttpRequestInternalVtable[0] = (void*) &LinuxHttpRequestInternal::destroy;
    linuxHttpRequestInternalVtable[1] = (void*) &LinuxHttpRequestInternal::destroy;

    if (workaroundAMD) {/*
        patchOff = (unsigned int) hybris_dlsym(handle, "_ZN21BlockTessallatorCache5resetER11BlockSourceRK8BlockPos") +
                   (0x40AD97 - 0x40ACD0);
        for (unsigned int i = 0; i < 0x40ADA0 - 0x40AD97; i++)
            ((char *) (void *) patchOff)[i] = 0x90;*/
        patchOff = (unsigned int) hybris_dlsym(handle, "_ZN21BlockTessallatorCache5resetER11BlockSourceRK8BlockPos") + (0x40AD9B - 0x40ACD0);
        patchCallInstruction((void*) patchOff, (void*) &pshufb_xmm4_xmm0, false);
    }

    std::cout << "patches applied!\n";

    // load symbols for gl
    gl::getOpenGLVendor = (std::string (*)()) hybris_dlsym(handle, "_ZN2gl15getOpenGLVendorEv");
    gl::getOpenGLRenderer = (std::string (*)()) hybris_dlsym(handle, "_ZN2gl17getOpenGLRendererEv");
    gl::getOpenGLVersion = (std::string (*)()) hybris_dlsym(handle, "_ZN2gl16getOpenGLVersionEv");
    gl::getOpenGLExtensions = (std::string (*)()) hybris_dlsym(handle, "_ZN2gl19getOpenGLExtensionsEv");
    mce::Platform::OGL::OGL_initBindings = (void (*)()) hybris_dlsym(handle, "_ZN3mce8Platform3OGL12InitBindingsEv");

    // init linux app platform
    AppPlatform::myVtable = (void**) hybris_dlsym(handle, "_ZTV11AppPlatform");
    AppPlatform::_singleton = (AppPlatform**) hybris_dlsym(handle, "_ZN11AppPlatform10mSingletonE");
    AppPlatform::AppPlatform_construct = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatformC2Ev");
    AppPlatform::AppPlatform_initialize = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatform10initializeEv");
    AppPlatform::AppPlatform__fireAppFocusGained = (void (*)(AppPlatform*)) hybris_dlsym(handle, "_ZN11AppPlatform19_fireAppFocusGainedEv");

    void** ptr = (void**) hybris_dlsym(handle, "_ZN9crossplat3JVME");
    *ptr = (void*) 1; // this just needs not to be null

    std::cout << "init app platform vtable\n";
    LinuxAppPlatform::initVtable(handle);
    std::cout << "init app platform\n";
    LinuxAppPlatform* platform = new LinuxAppPlatform();
    std::cout << "app platform initialized\n";

    Mouse::feed = (void (*)(char, char, short, short, short, short)) hybris_dlsym(handle, "_ZN5Mouse4feedEccssss");

    Keyboard::inputs = (std::vector<KeyboardAction>*) hybris_dlsym(handle, "_ZN8Keyboard7_inputsE");
    Keyboard::states = (int*) hybris_dlsym(handle, "_ZN8Keyboard7_statesE");
    Keyboard::Keyboard_feedText = (void (*)(const std::string&, bool, unsigned char)) hybris_dlsym(handle, "_ZN8Keyboard8feedTextERKSsbh");

    Options::Options_getFullscreen = (bool (*)(Options*)) hybris_dlsym(handle, "_ZNK7Options13getFullscreenEv");
    Options::Options_setFullscreen = (void (*)(Options*, bool)) hybris_dlsym(handle, "_ZN7Options13setFullscreenEb");

    std::cout << "init window\n";
    eglutInitWindowSize(windowWidth, windowHeight);
    eglutInitAPIMask(EGLUT_OPENGL_ES2_BIT);
    eglutInit(argc, argv);

    winId = eglutCreateWindow("Minecraft");

    // init MinecraftClient
    App::App_init = (void (*)(App*, AppContext&)) hybris_dlsym(handle, "_ZN3App4initER10AppContext");
    MinecraftClient::MinecraftClient_construct = (void (*)(MinecraftClient*, int, char**)) hybris_dlsym(handle, "_ZN15MinecraftClientC2EiPPc");
    MinecraftClient::MinecraftClient_update = (void (*)(MinecraftClient*)) hybris_dlsym(handle, "_ZN15MinecraftClient6updateEv");
    MinecraftClient::MinecraftClient_setRenderingSize = (void (*)(MinecraftClient*, int, int)) hybris_dlsym(handle, "_ZN15MinecraftClient16setRenderingSizeEii");
    MinecraftClient::MinecraftClient_setUISizeAndScale = (void (*)(MinecraftClient*, int, int, float)) hybris_dlsym(handle, "_ZN15MinecraftClient17setUISizeAndScaleEiif");
    MinecraftClient::MinecraftClient_getOptions = (Options* (*)(MinecraftClient*)) hybris_dlsym(handle, "_ZN15MinecraftClient10getOptionsEv");
    AppContext ctx;
    ctx.platform = platform;
    ctx.doRender = true;

    platform->initialize();

    mce::Platform::OGL::initBindings();

    std::cout << "create minecraft client\n";
    client = new MinecraftClient(argc, argv);
    std::cout << "init minecraft client\n";
    client->init(ctx);
    std::cout << "initialized lib\n";

    if (client->getOptions()->getFullscreen())
        eglutToggleFullscreen();

    for (void* mod : mods) {
        void (*initFunc)(MinecraftClient*) = (void (*)(MinecraftClient*)) hybris_dlsym(mod, "mod_set_minecraft");
        if ((void*) initFunc != nullptr)
            initFunc(client);
    }

    eglutIdleFunc(minecraft_idle);
    eglutReshapeFunc(minecraft_reshape);
    eglutDisplayFunc(minecraft_draw);
    eglutMouseFunc(minecraft_mouse);
    eglutMouseButtonFunc(minecraft_mouse_button);
    eglutKeyboardFunc(minecraft_keyboard);
    eglutSpecialFunc(minecraft_keyboard_special);
    eglutCloseWindowFunc(minecraft_close);
    std::cout << "initialized display\n";

    // init
    //(*AppPlatform::_singleton)->_fireAppFocusGained();
    client->setRenderingSize(windowWidth, windowHeight);
    client->setUISizeAndScale(windowWidth, windowHeight, pixelSize);
    eglutMainLoop();
    return 0;
}
