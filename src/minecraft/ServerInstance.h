#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include "string.h"
#include "LevelSettings.h"

class IMinecraftApp;
class Minecraft;
class Whitelist;
class OpsList;
namespace minecraft { namespace api { class Api; } }
namespace mce { class UUID; }
class MinecraftEventing;
class ResourcePackRepository;
class ResourcePackManager;
class ContentTierManager;
class FilePathManager;
class IContentAccessibilityProvider;

class NetworkHandler {

public:

    char filler[0x200];

    NetworkHandler();

};

class ServerInstance {

public:

    char filler[0x8];
    Minecraft* minecraft;
    char filler2[0x200];

    /// @symbol _ZN14ServerInstanceC2ER13IMinecraftAppRK9WhitelistRK7OpsListP15FilePathManagerNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSsSsRK19IContentKeyProviderSs13LevelSettingsRN9minecraft3api3ApiEibiiibRKSt6vectorISsSaISsEESsRKN3mce4UUIDER17MinecraftEventingR14NetworkHandlerR22ResourcePackRepositoryRK18ContentTierManagerR19ResourcePackManagerPS15_St8functionIFvRKSsEE
    ServerInstance(IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, NetworkHandler&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>);

    ~ServerInstance();

    void update();

    void startLeaveGame();

    bool isLeaveGameDone() const;

    void mainThreadNetworkUpdate_HACK();

};