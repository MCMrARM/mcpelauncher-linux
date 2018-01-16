#include "modloader.h"
#include "log.h"
#include <hybris/dlfcn.h>
#include <dirent.h>

void* ModLoader::loadMod(std::string const& path) {
    void* handle = hybris_dlopen(path.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        Log::error("ModLoader", "Failed to load mod: %s", path.c_str());
        return nullptr;
    }

    void (*initFunc)();
    initFunc = (void (*)()) hybris_dlsym(handle, "mod_init");
    if (((void*) initFunc) == nullptr) {
        Log::warn("ModLoader", "Mod %s does not have an init function", path.c_str());
        return handle;
    }
    initFunc();

    return handle;
}

void ModLoader::loadModsFromDirectory(std::string const& path) {
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir(path.c_str())) != NULL) {
        Log::info("ModLoader", "Loading mods");
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_name[0] == '.')
                continue;
            std::string fileName(ent->d_name);
            int len = fileName.length();
            if (len < 4 || fileName[len - 3] != '.' || fileName[len - 2] != 's' || fileName[len - 1] != 'o')
                continue;
            Log::info("ModLoader", "Loading mod: %s", fileName);
            void* mod = loadMod(path + fileName);
            if (mod != nullptr)
                mods.push_back(mod);
        }
        closedir(dir);
        Log::info("ModLoader", "Loaded %li mods", mods.size());
    }
}

void ModLoader::onGameInitialized(MinecraftGame* game) {
    if (mods.empty())
        return;
    Log::info("ModLoader", "Initializing mods");
    for (void* mod : mods) {
        void (*initFunc)(MinecraftGame*) = (void (*)(MinecraftGame*)) hybris_dlsym(mod, "mod_set_minecraft");
        if ((void*) initFunc != nullptr)
            initFunc(game);
    }
}