#include "common.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <sys/param.h>
#include <unistd.h>
#include <cxxabi.h>
#include <execinfo.h>
#include <malloc.h>

extern "C" {
#include "../hybris/include/hybris/hook.h"
#include "../hybris/include/hybris/dlfcn.h"
}

std::string getCWD() {
    char _cwd[MAXPATHLEN];
    getcwd(_cwd, MAXPATHLEN);
    return std::string(_cwd) + "/";
}

bool loadLibrary(std::string path) {
    void* handle = hybris_dlopen((getCWD() + "libs/" + path).c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        printf("failed to load library %s: %s\n", path.c_str(), hybris_dlerror());
        return false;
    }
    return true;
}

void* loadLibraryOS(std::string path, const char** symbols) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        printf("failed to load library %s: %s\n", path.c_str(), dlerror());
        return nullptr;
    }
    printf("oslib: %s: %i\n", path.c_str(), (int) handle);
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

void* loadMod(std::string path) {
    void* handle = hybris_dlopen((getCWD() + "mods/" + path).c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        printf("failed to load mod: %s\n", path.c_str());
        return nullptr;
    }

    void (*initFunc)();
    initFunc = (void (*)()) hybris_dlsym(handle, "mod_init");
    if (((void*) initFunc) == nullptr) {
        printf("warn: mod %s doesn't have a init function\n", path.c_str());
        return handle;
    }
    initFunc();

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

void __android_log_vprint(int prio, const char *tag, const char *fmt, va_list args) {
    printf("[%s] ", tag);
    vprintf(fmt, args);
    printf("\n");
}
void __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    __android_log_vprint(prio, tag, fmt, args);
    va_end(args);
}
void __android_log_write(int prio, const char *tag, const char *text) {
    printf("[%s] %s\n", tag, text);
}

void hookAndroidLog() {
    hybris_hook("__android_log_print", (void*) __android_log_print);
    hybris_hook("__android_log_vprint", (void*) __android_log_vprint);
    hybris_hook("__android_log_write", (void*) __android_log_write);
}

void patchCallInstruction(void* patchOff, void* func, bool jump) {
    unsigned char* data = (unsigned char*) patchOff;
    printf("original: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);
    data[0] = (unsigned char) (jump ? 0xe9 : 0xe8);
    int ptr = ((int) func) - (int) patchOff - 5;
    memcpy(&data[1], &ptr, sizeof(int));
    printf("post patch: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);
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