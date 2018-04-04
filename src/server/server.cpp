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
#include "../common/modloader.h"
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
#include "../minecraft/SaveTransactionManager.h"
#include "server_minecraft_app.h"
#include "server_properties.h"
#include "../common/hook.h"
#include "../minecraft/Resource.h"
#include "../minecraft/AppResourceLoader.h"
#include "../common/extract.h"
#include "stub_key_provider.h"

extern "C" {
#include <hybris/dlfcn.h>
#include <hybris/hook.h>
#include "../../libs/hybris/src/jb/linker.h"
}

void stubFunc() {
    Log::warn("Launcher", "Stubbed function call");
}

int main(int argc, char *argv[]) {
    if (argc == 3 && strcmp(argv[1], "extract") == 0) {
        ExtractHelper::extractApk(argv[2], PathHelper::getWorkingDir());
        return 0;
    }

    registerCrashHandler();

    ServerProperties properties;
    {
        std::ifstream propertiesFile("server.properties");
        if (propertiesFile) {
            properties.load(propertiesFile);
        } else {
            try {
                propertiesFile.open(PathHelper::findDataFile("server.properties"));
                if (propertiesFile)
                    properties.load(propertiesFile);
            } catch (std::runtime_error& e) {
            }
        }
    }

    Log::trace("Launcher", "Loading hybris libraries");
    stubSymbols(android_symbols, (void*) stubFunc);
    stubSymbols(egl_symbols, (void*) stubFunc);
    stubSymbols(fmod_symbols, (void*) stubFunc);
    hybris_hook("eglGetProcAddress", (void*) stubFunc);
    hybris_hook("mcpelauncher_hook", (void*) hookFunction);
#ifdef __APPLE__
    void* libmLib = loadLibraryOS("libm.dylib", libm_symbols);
#else
    void* libmLib = loadLibraryOS("libm.so.6", libm_symbols);
#endif
    hookAndroidLog();
    if (!load_empty_library("libc.so") || !load_empty_library("libm.so"))
        return -1;
    // load stub libraries
    if (!load_empty_library("libandroid.so") || !load_empty_library("liblog.so") || !load_empty_library("libEGL.so") || !load_empty_library("libGLESv2.so") || !load_empty_library("libOpenSLES.so") || !load_empty_library("libfmod.so") || !load_empty_library("libGLESv1_CM.so"))
        return -1;
    if (!load_empty_library("libmcpelauncher_mod.so"))
        return -1;
    load_empty_library("libstdc++.so");
    Log::trace("Launcher", "Loading Minecraft library");
    std::string mcpePath = PathHelper::findDataFile("libs/libminecraftpe.so");
    void* handle = hybris_dlopen(mcpePath.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error("Launcher", "Failed to load Minecraft: %s", hybris_dlerror());
        return -1;
    }
    addHookLibrary(handle, mcpePath);

    unsigned int libBase = ((soinfo*) handle)->base;
    Log::info("Launcher", "Loaded Minecraft library");
    Log::debug("Launcher", "Minecraft is at offset 0x%x", libBase);

    minecraft_symbols_init(handle);

    mcpe::string::empty = (mcpe::string*) hybris_dlsym(handle, "_ZN4Util12EMPTY_STRINGE");

    Log::info("Launcher", "Applying patches");
    void* ptr = hybris_dlsym(handle, "_ZN5Level17_checkUserStorageEv");
    patchCallInstruction(ptr, (void*) (void (*)()) []{ }, true);

    ModLoader modLoader;
    modLoader.loadModsFromDirectory(PathHelper::getPrimaryDataDirectory() + "mods/");

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
    api.envPath = PathHelper::getWorkingDir();
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
    FilePathManager pathmgr (platform->getCurrentStoragePath(), false);
    pathmgr.setPackagePath(platform->getPackagePath());
    pathmgr.setSettingsPath(pathmgr.getRootPath());
    Log::trace("Launcher", "Initializing resource loaders");
    Resource::registerLoader((ResourceFileSystem) 1, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return pathmgr.getPackagePath(); })));
    // Resource::registerLoader((ResourceFileSystem) 7, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return pathmgr.getDataUrl(); })));
    Resource::registerLoader((ResourceFileSystem) 8, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return pathmgr.getUserDataPath(); })));
    Resource::registerLoader((ResourceFileSystem) 4, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return pathmgr.getSettingsPath(); })));
    // Resource::registerLoader((ResourceFileSystem) 5, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return pathmgr.getExternalFilePath(); })));
    // Resource::registerLoader((ResourceFileSystem) 2, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return ""; })));
    // Resource::registerLoader((ResourceFileSystem) 3, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return ""; })));
    // Resource::registerLoader((ResourceFileSystem) 9, std::unique_ptr<ResourceLoader>(new ScreenshotLoader));
    // Resource::registerLoader((ResourceFileSystem) 0xA, std::unique_ptr<ResourceLoader>(new AppResourceLoader([&pathmgr] { return ""; })));

    Log::trace("Launcher", "Initializing MinecraftEventing (create instance)");
    MinecraftEventing eventing (pathmgr.getRootPath());
    /*Log::trace("Launcher", "Social::UserManager::CreateUserManager()");
    auto userManager = Social::UserManager::CreateUserManager();*/
    Log::trace("Launcher", "Initializing MinecraftEventing (init call)");
    eventing.init();
    Log::trace("Launcher", "Initializing ResourcePackManager");
    ContentTierManager ctm;
    ResourcePackManager* resourcePackManager = new ResourcePackManager([&pathmgr]() { return pathmgr.getRootPath(); }, ctm);
    Resource::registerLoader((ResourceFileSystem) 0, std::unique_ptr<ResourceLoader>(resourcePackManager));
    Log::trace("Launcher", "Initializing PackManifestFactory");
    PackManifestFactory packManifestFactory (eventing);
    Log::trace("Launcher", "Initializing SkinPackKeyProvider");
    SkinPackKeyProvider skinPackKeyProvider;
    Log::trace("Launcher", "Initializing StubKeyProvider");
    StubKeyProvider stubKeyProvider;
    Log::trace("Launcher", "Initializing PackSourceFactory");
    PackSourceFactory packSourceFactory (nullptr);
    Log::trace("Launcher", "Initializing ResourcePackRepository");
    ResourcePackRepository resourcePackRepo (eventing, packManifestFactory, skinPackKeyProvider, &pathmgr, packSourceFactory);
    Log::trace("Launcher", "Adding vanilla resource pack");
    std::unique_ptr<ResourcePackStack> stack (new ResourcePackStack());
    stack->add(PackInstance(resourcePackRepo.vanillaPack, -1, false, nullptr), resourcePackRepo, false);
    resourcePackManager->setStack(std::move(stack), (ResourcePackStackType) 3, false);
    Log::trace("Launcher", "Adding world resource packs");
    resourcePackRepo.addWorldResourcePacks(pathmgr.getWorldsPath().std() + properties.getString("level-dir"));
    Log::trace("Launcher", "Initializing Automation::AutomationClient");
    DedicatedServerMinecraftApp minecraftApp;
    Automation::AutomationClient aclient (minecraftApp);
    minecraftApp.automationClient = &aclient;
    Log::debug("Launcher", "Initializing SaveTransactionManager");
    std::shared_ptr<SaveTransactionManager> saveTransactionManager (new SaveTransactionManager([](bool b) {
        if (b)
            Log::debug("Launcher", "Saving the world...");
        else
            Log::debug("Launcher", "World has been saved.");
    }));
    Log::debug("Launcher", "Initializing ServerInstance");
    auto idleTimeout = std::chrono::seconds((int) (properties.getFloat("player-idle-timeout", 0) * 60.f));
    IContentKeyProvider* keyProvider = &stubKeyProvider;
    // In an older version of the server launcher there was a bug that would cause the worlds to be encrypted with the
    // skin packs key. To allow those worlds to be ever loaded again, a server property is added.
    if (properties.getBool("level-skinpack-encrypted"))
        keyProvider = &skinPackKeyProvider;
    ServerInstance instance (minecraftApp, whitelist, ops, &pathmgr, idleTimeout, /* world dir */ properties.getString("level-dir"), /* world name */ properties.getString("level-name"), mcpe::string(), *keyProvider, properties.getString("motd"), /* settings */ levelSettings, api, properties.getInt("view-distance", 22), true, properties.getInt("server-port", 19132), properties.getInt("server-port-v6", 19133), properties.getInt("max-players", 20), properties.getBool("online-mode", true), {}, "normal", *mce::UUID::EMPTY, eventing, resourcePackRepo, ctm, saveTransactionManager, *resourcePackManager, nullptr, [](mcpe::string const& s) {
        std::cout << "??? " << s.c_str() << "\n";
    }, [](mcpe::string const& s) {
        Log::debug("Launcher", "Saving level: %s", s.c_str());
    });
    Log::trace("Launcher", "Loading language data");
    I18n::loadLanguages(*resourcePackManager, "en_US");
    resourcePackManager->onLanguageChanged();
    Log::info("Launcher", "Server initialized");
    modLoader.onServerInstanceInitialized(&instance);

    instance.startServerThread();

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
        Scheduler::client()->processCoroutines(std::chrono::duration_cast<std::chrono::duration<long long>>(tp2 - tp));
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
