#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include "string.h"

class IMinecraftApp;
class Minecraft;
class Whitelist;
class OpsList;
class LevelSettings;
namespace minecraft { namespace api { class Api; } }
namespace mce { class UUID; }
class MinecraftEventing;
class ResourcePackRepository;
class ResourcePackManager;
class ContentTierManager;

class NetworkHandler {

public:
    static void (*NetworkHandler_construct)(NetworkHandler*);

    char filler[0x200];

    NetworkHandler() {
        NetworkHandler_construct(this);
    }

};

class ServerInstance {

public:

    char filler[0x8];
    Minecraft* minecraft;
    char filler2[0x200];

    static void (*ServerInstance_construct)(ServerInstance*, IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, NetworkHandler&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>);
    static void (*ServerInstance_update)(ServerInstance*);
    static void (*ServerInstance_mainThreadNetworkUpdate_HACK)(ServerInstance*);

    void update() {
        ServerInstance_update(this);
    }

    void mainThreadNetworkUpdate_HACK() {
        ServerInstance_mainThreadNetworkUpdate_HACK(this);
    }

};