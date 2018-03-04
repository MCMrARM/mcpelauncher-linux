#include "common.h"
#include "path_helper.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <csignal>
#include <cstdlib>
#include "log.h"

extern "C" {
#include <hybris/hook.h>
#include <hybris/dlfcn.h>
}

static const char* TAG = "Launcher";

bool loadLibrary(std::string path) {
    void* handle = hybris_dlopen(PathHelper::findDataFile("libs/hybris/" + path).c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error(TAG, "Failed to load hybris library %s: %s", path.c_str(), hybris_dlerror());
        return false;
    }
    return true;
}

void* loadLibraryOS(std::string path, const char** symbols) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error(TAG, "Failed to load OS library %s", path.c_str());
        return nullptr;
    }
    Log::trace(TAG, "Loaded OS library %s", path.c_str());
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

typedef enum AndroidLogPriority {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT
} android_LogPriority;

static LogLevel convertAndroidLogLevel(int level) {
    if (level <= AndroidLogPriority::ANDROID_LOG_VERBOSE)
        return LogLevel::LOG_TRACE;
    if (level == AndroidLogPriority::ANDROID_LOG_DEBUG)
        return LogLevel::LOG_DEBUG;
    if (level == AndroidLogPriority::ANDROID_LOG_INFO)
        return LogLevel::LOG_INFO;
    if (level == AndroidLogPriority::ANDROID_LOG_WARN)
        return LogLevel::LOG_WARN;
    if (level >= AndroidLogPriority::ANDROID_LOG_ERROR)
        return LogLevel::LOG_ERROR;
    return LogLevel::LOG_ERROR;
}
void __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args) {
    Log::vlog(convertAndroidLogLevel(prio), tag, fmt, args);
}
void __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Log::vlog(convertAndroidLogLevel(prio), tag, fmt, args);
    va_end(args);
}
void __android_log_write(int prio, const char *tag, const char *text) {
    Log::log(convertAndroidLogLevel(prio), tag, "%s", text);
}

void hookAndroidLog() {
    hybris_hook("__android_log_print", (void*) __android_log_print);
    hybris_hook("__android_log_vprint", (void*) __android_log_vprint);
    hybris_hook("__android_log_write", (void*) __android_log_write);
}

void patchCallInstruction(void* patchOff, void* func, bool jump) {
    unsigned char* data = (unsigned char*) patchOff;
    Log::trace(TAG, "Patching - original: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
    data[0] = (unsigned char) (jump ? 0xe9 : 0xe8);
    int ptr = ((int) func) - (int) patchOff - 5;
    memcpy(&data[1], &ptr, sizeof(int));
    Log::trace(TAG, "Patching - result: %i %i %i %i %i", data[0], data[1], data[2], data[3], data[4]);
}

bool hasCrashed = false;
void handleSignal(int signal, void* aptr) {
    printf("Signal %i received\n", signal);
    if (hasCrashed)
        return;
    hasCrashed = true;
    void** ptr = &aptr;
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
    printf("Dumping stack...\n");
    for (int i = 0; i < 1000; i++) {
        void* pptr = *ptr;
        Dl_info symInfo;
        if (hybris_dladdr(pptr, &symInfo) && symInfo.dli_sname != nullptr && strlen(symInfo.dli_sname) > 0) {
            int status = 0;
            nameBuf = abi::__cxa_demangle(symInfo.dli_sname, nameBuf, &nameBufLen, &status);
            printf("#%i HYBRIS %s+%i in %s+0x%04x [0x%04x]\n", i, nameBuf, (unsigned int) pptr - (unsigned int) symInfo.dli_saddr, symInfo.dli_fname, (unsigned int) pptr - (unsigned int) symInfo.dli_fbase, (int)pptr);
        }
        ptr++;
    }
    abort();
}

void registerCrashHandler() {
    struct sigaction act;
    act.sa_handler = (void (*)(int)) handleSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGSEGV, &act, 0);
    sigaction(SIGABRT, &act, 0);
}

void workerPoolDestroy(void* th) {
    Log::trace("Launcher", "WorkerPool-related class destroy %uli", (unsigned long) th);
}
void workaroundShutdownCrash(void* handle) {
    // this is an ugly hack to workaround the close app crashes MCPE causes
    unsigned int patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9TaskGroupD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);
    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN10WorkerPoolD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);
    patchOff = (unsigned int) hybris_dlsym(handle, "_ZN9SchedulerD2Ev");
    patchCallInstruction((void*) patchOff, (void*) &workerPoolDestroy, true);
}
