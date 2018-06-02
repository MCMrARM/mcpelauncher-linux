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
class LevelStorage;
class Scheduler;
class LevelData;

class ServerInstance {

public:

    char filler[0x10];
    Minecraft* minecraft;
    char filler2[0x200];

    /// @symbol _ZN14ServerInstanceC2ER13IMinecraftAppR9WhitelistRK7OpsListP15FilePathManagerNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSsSs13LevelSettingsRN9minecraft3api3ApiEibiiibRKSt6vectorISsSaISsEESsRKN3mce4UUIDER17MinecraftEventingR22ResourcePackRepositoryRK18ContentTierManagerR19ResourcePackManagerSt8functionIFSt10unique_ptrI12LevelStorageSt14default_deleteIS13_EER9SchedulerEERKSsP9LevelDataPSZ_S11_IFvS1C_EES1H_
    ServerInstance(IMinecraftApp&, Whitelist&, OpsList const&, FilePathManager*, std::chrono::seconds, mcpe::string, mcpe::string, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, std::function<std::unique_ptr<LevelStorage> (Scheduler&)>, mcpe::string const&, LevelData*, ResourcePackManager*, std::function<void (mcpe::string const&)>, std::function<void (mcpe::string const&)>);

    ~ServerInstance();

    void startServerThread();

    void leaveGameSync();

    /// @symbol _ZN14ServerInstance20queueForServerThreadESt8functionIFvvEE
    void queueForServerThread(std::function<void ()>);

};