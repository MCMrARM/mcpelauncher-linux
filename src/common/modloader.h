#pragma once

#include <vector>
#include <string>
#include <set>

class MinecraftGame;
class ServerInstance;

class ModLoader {

private:
    std::vector<void*> mods;

    std::vector<std::string> getModDependencies(std::string const& path);

    void loadModMulti(std::string const& path, std::string const& fileName, std::set<std::string>& otherMods);

public:
    void* loadMod(std::string const& path);

    void loadModsFromDirectory(std::string const& path);

    void onGameInitialized(MinecraftGame* game);

    void onServerInstanceInitialized(ServerInstance* server);

};