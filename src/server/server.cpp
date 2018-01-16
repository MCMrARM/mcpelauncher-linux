#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <fcntl.h>
#include <thread>
#include "../common/symbols/android_symbols.h"
#include "../common/symbols/egl_symbols.h"
#include "../common/symbols/fmod_symbols.h"
#include "../common/symbols/libm_symbols.h"
#include "../common/common.h"
#include "../common/log.h"
#include "../client/appplatform.h"
#include "../minecraft/symbols.h"
#include "../minecraft/Api.h"
#include "../minecraft/Whitelist.h"
#include "../minecraft/OpsList.h"
#include "../minecraft/ResourcePack.h"
#include "../minecraft/FilePathManager.h"
#include "../minecraft/MinecraftEventing.h"
#include "../minecraft/UUID.h"
#include "../minecraft/LevelSettings.h"
#include "../minecraft/ServerInstance.h"
#include "../minecraft/UserManager.h"
#include "../minecraft/AutomationClient.h"
#include "../minecraft/Scheduler.h"
#include "../minecraft/Minecraft.h"
#include "../minecraft/MinecraftCommands.h"
#include "../minecraft/DedicatedServerCommandOrigin.h"
#include "../minecraft/CommandOutputSender.h"
#include "../minecraft/CommandOutput.h"
#include "../minecraft/I18n.h"
#include "../minecraft/ResourcePackStack.h"
#include "server_minecraft_app.h"
#include "server_properties.h"

extern "C" {
#include <hybris/dlfcn.h>
#include <hybris/hook.h>
#include "../../libs/hybris/src/jb/linker.h"
}

void stubFunc() {
    Log::warn("Launcher", "Stubbed function call");
}

int main(int argc, char *argv[]) {
    registerCrashHandler();

    // We're going to look at the CWD instead of the proper assets folders because to support other paths I'd likely
    // have to register a proper asset loader in MCPE, and the default one just falls back to the current directory for
    // assets, so let's at least use the CWD for as much stuff as possible.
    std::string cwd = PathHelper::getWorkingDir();

    ServerProperties properties;
    {
        std::ifstream propertiesFile("server.properties");
        if (propertiesFile)
            properties.load(propertiesFile);
    }

    Log::trace("Launcher", "Loading hybris libraries");
    stubSymbols(android_symbols, (void*) stubFunc);
    stubSymbols(egl_symbols, (void*) stubFunc);
    stubSymbols(fmod_symbols, (void*) stubFunc);
    hybris_hook("eglGetProcAddress", (void*) stubFunc);
    void* libmLib = loadLibraryOS("libm.so.6", libm_symbols);
    hookAndroidLog();
    if (!load_empty_library("libc.so") || !load_empty_library("libm.so"))
        return -1;
    // load stub libraries
    if (!load_empty_library("libandroid.so") || !load_empty_library("liblog.so") || !load_empty_library("libEGL.so") || !load_empty_library("libGLESv2.so") || !load_empty_library("libOpenSLES.so") || !load_empty_library("libfmod.so") || !load_empty_library("libGLESv1_CM.so"))
        return -1;
    if (!load_empty_library("libmcpelauncher_mod.so"))
        return -1;
    Log::trace("Launcher", "Loading Minecraft library");
    void* handle = hybris_dlopen((cwd + "libs/libminecraftpe.so").c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error("Launcher", "Failed to load Minecraft: %s", hybris_dlerror());
        return -1;
    }

    unsigned int libBase = ((soinfo*) handle)->base;
    Log::info("Launcher", "Loaded Minecraft library");
    Log::debug("Launcher", "Minecraft is at offset 0x%x", libBase);

    minecraft_symbols_init(handle);

    mcpe::string::empty = (mcpe::string*) hybris_dlsym(handle, "_ZN4Util12EMPTY_STRINGE");

    Log::info("Launcher", "Starting server initialization");

    Log::trace("Launcher", "Initializing AppPlatform (vtable)");
    LinuxAppPlatform::initVtable(handle);
    Log::trace("Launcher", "Initializing AppPlatform (create instance)");
    LinuxAppPlatform* platform = new LinuxAppPlatform();
    Log::trace("Launcher", "Initializing AppPlatform (initialize call)");
    platform->initialize();
    Log::debug("Launcher", "AppPlatform initialized successfully");

    Log::trace("Launcher", "Loading whitelist and operator list");
    Whitelist whitelist;
    OpsList ops (true);
    Log::trace("Launcher", "Initializing Minecraft API classes");
    minecraft::api::Api api;
    api.vtable = (void**) hybris_dlsym(handle, "_ZTVN9minecraft3api3ApiE") + 2;
    api.envPath = cwd;
    api.playerIfaceVtable = (void**) hybris_dlsym(handle, "_ZTVN9minecraft3api15PlayerInterfaceE") + 2;
    api.entityIfaceVtable = (void**) hybris_dlsym(handle, "_ZTVN9minecraft3api15EntityInterfaceE") + 2;
    api.networkIfaceVtable = (void**) hybris_dlsym(handle, "_ZTVN9minecraft3api16NetworkInterfaceE") + 2;
    api.playerInteractionsIfaceVtable = (void**) hybris_dlsym(handle, "_ZTVN9minecraft3api26PlayerInteractionInterfaceE") + 2;

    Log::trace("Launcher", "Setting up level settings");
    LevelSettings levelSettings;
    levelSettings.seed = properties.getInt("level-seed", 0);
    levelSettings.gametype = properties.getInt("gamemode", 0);
    levelSettings.forceGameType = properties.getBool("force-gamemode", false);
    levelSettings.difficulty = properties.getInt("difficulty", 0);
    levelSettings.dimension = 0;
    levelSettings.generator = properties.getInt("level-generator", 1);
    levelSettings.edu = false;
    levelSettings.mpGame = true;
    levelSettings.lanBroadcast = true;
    levelSettings.commandsEnabled = true;
    levelSettings.texturepacksRequired = false;

    Log::trace("Launcher", "Initializing FilePathManager");
    FilePathManager pathmgr (cwd, false);
    Log::trace("Launcher", "Initializing MinecraftEventing (create instance)");
    MinecraftEventing eventing (cwd);
    /*Log::trace("Launcher", "Social::UserManager::CreateUserManager()");
    auto userManager = Social::UserManager::CreateUserManager();*/
    Log::trace("Launcher", "Initializing MinecraftEventing (init call)");
    eventing.init();
    Log::trace("Launcher", "Initializing ResourcePackManager");
    ContentTierManager ctm;
    ResourcePackManager resourcePackManager ([cwd]() {
        return cwd;
    }, ctm);
    Log::trace("Launcher", "Initializing PackManifestFactory");
    PackManifestFactory packManifestFactory (eventing);
    Log::trace("Launcher", "Initializing SkinPackKeyProvider");
    SkinPackKeyProvider skinPackKeyProvider;
    Log::trace("Launcher", "Initializing PackSourceFactory");
    PackSourceFactory packSourceFactory (nullptr);
    Log::trace("Launcher", "Initializing ResourcePackRepository");
    ResourcePackRepository resourcePackRepo (eventing, packManifestFactory, skinPackKeyProvider, &pathmgr, packSourceFactory);
    Log::trace("Launcher", "Adding vanilla resource pack");
    std::unique_ptr<ResourcePackStack> stack (new ResourcePackStack());
    stack->add(PackInstance(resourcePackRepo.vanillaPack, -1, false), resourcePackRepo, false);
    resourcePackManager.setStack(std::move(stack), (ResourcePackStackType) 3, false);
    Log::trace("Launcher", "Initializing NetworkHandler");
    NetworkHandler handler;
    Log::trace("Launcher", "Initializing Automation::AutomationClient");
    DedicatedServerMinecraftApp minecraftApp;
    Automation::AutomationClient aclient (minecraftApp);
    minecraftApp.automationClient = &aclient;
    Log::debug("Launcher", "Initializing ServerInstance");
    ServerInstance instance (minecraftApp, whitelist, ops, &pathmgr, std::chrono::duration_cast<std::chrono::duration<long long>>(std::chrono::milliseconds(50)), /* world dir */ properties.getString("level-dir"), /* world name */ properties.getString("level-name"), mcpe::string(), skinPackKeyProvider, properties.getString("motd"), /* settings */ levelSettings, api, properties.getInt("view-distance", 22), true, /* (query?) port */ properties.getInt("server-port", 19132), /* (maybe not) port */ 19132, properties.getInt("max-players", 20), properties.getBool("online-mode", true), {}, "normal", *mce::UUID::EMPTY, eventing, handler, resourcePackRepo, ctm, resourcePackManager, nullptr, [](mcpe::string const& s) {
        std::cout << "??? " << s.c_str() << "\n";
    });
    Log::trace("Launcher", "Loading language data");
    I18n::loadLanguages(resourcePackManager, nullptr, "en_US");
    resourcePackManager.onLanguageChanged();
    Log::info("Launcher", "Server initialized");

    int flags = fcntl(0, F_GETFL, 0);
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    char lineBuffer[1024 * 16];
    size_t lineBufferOffset = 0;

    static bool isInterrupted = false;
    signal(SIGINT, [](int i) {
        isInterrupted = true;
    });

    auto tp = std::chrono::steady_clock::now();
    int updatesPerSecond = 25;
    while (!isInterrupted) {
        ssize_t r;
        while ((r = read(0, &lineBuffer[lineBufferOffset], sizeof(lineBuffer) - lineBufferOffset)) > 0)
            lineBufferOffset += r;
        for (size_t i = 0; i < lineBufferOffset; ) {
            if (i == sizeof(lineBuffer) - 1 || lineBuffer[i] == '\n') {
                std::string cmd = std::string(lineBuffer, i);
                memcpy(lineBuffer, &lineBuffer[i + 1], lineBufferOffset - i - 1);
                lineBufferOffset -= i + 1;

                std::unique_ptr<DedicatedServerCommandOrigin> commandOrigin(new DedicatedServerCommandOrigin("Server", *instance.minecraft));
                instance.minecraft->getCommands()->requestCommandExecution(std::move(commandOrigin), cmd, 4, true);
                i = 0;
            } else {
                i++;
            }
        }

        auto tp2 = std::chrono::steady_clock::now();
        instance.update();
        instance.mainThreadNetworkUpdate_HACK();
        Scheduler::singleton()->processCoroutines(std::chrono::duration_cast<std::chrono::duration<long long>>(tp2 - tp));
        std::this_thread::sleep_until(tp2 + std::chrono::nanoseconds(1000000000L / updatesPerSecond));
        tp = tp2;
    }

    Log::info("Launcher", "Stopping...");
    instance.startLeaveGame();
    while (!instance.isLeaveGameDone())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    workaroundShutdownCrash(handle);
    return 0;
}
