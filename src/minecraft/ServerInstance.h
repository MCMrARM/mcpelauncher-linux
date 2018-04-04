#pragma once

#include <chrono>
#include <vector>
#include <string>
#include <functional>
#include "std/string.h"
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
class IContentKeyProvider;
class SaveTransactionManager;

class ServerInstance {

public:

    char filler[0x8];
    Minecraft* minecraft;
    char filler2[0x200];

    /// @symbol _ZN14ServerInstanceC2ER13IMinecraftAppRK9WhitelistRK7OpsListP15FilePathManagerNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSsSsRK19IContentKeyProviderSs13LevelSettingsRN9minecraft3api3ApiEibiiibRKSt6vectorISsSaISsEESsRKN3mce4UUIDER17MinecraftEventingR22ResourcePackRepositoryRK18ContentTierManagerSt10shared_ptrI22SaveTransactionManagerER19ResourcePackManagerPS16_St8functionIFvRKSsEES1D_
    ServerInstance(IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::seconds, mcpe::string, mcpe::string, mcpe::string, IContentKeyProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, ResourcePackRepository&, ContentTierManager const&, std::shared_ptr<SaveTransactionManager>, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>, std::function<void (mcpe::string const&)>);

    ~ServerInstance();

    void startServerThread();

    void startLeaveGame();

    bool isLeaveGameDone() const;

};