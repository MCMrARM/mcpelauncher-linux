#pragma once

#include <vector>
#include <string>

class MinecraftGame;

class ModLoader {

private:
    std::vector<void*> mods;

public:
    void* loadMod(std::string const& path);

    void loadModsFromDirectory(std::string const& path);

    void onGameInitialized(MinecraftGame* game);

};