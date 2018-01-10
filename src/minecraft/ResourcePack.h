#pragma once

#include <string>
#include <functional>
class MinecraftEventing;
class IPackTelemetry;
class FilePathManager;
class Options;
class ResourcePack;
class ResourcePackStack;
enum class ResourcePackStackType;

class IContentAccessibilityProvider {
    //
};

class SkinPackKeyProvider : public IContentAccessibilityProvider {

public:

    int filler;

    SkinPackKeyProvider();


};

class PackManifestFactory {

public:

    char filler[4];

    PackManifestFactory(IPackTelemetry&);

};

class PackSourceFactory {

public:

    char filler[0x100];

    PackSourceFactory(Options*);

};

class ResourcePackRepository {

public:

    char filler[0x28];
    ResourcePack* vanillaPack;
    char filler2[0x100];

    ResourcePackRepository(MinecraftEventing&, PackManifestFactory&, IContentAccessibilityProvider&, FilePathManager*, PackSourceFactory&);

};

struct ContentTierManager {

public:

    int filler;

    ContentTierManager();

};

class ResourcePackManager {

public:

    char filler[0x100];

    static void (*ResourcePackManager_construct)(ResourcePackManager*, std::function<mcpe::string ()> const&, ContentTierManager const&);

    ResourcePackManager(std::function<std::string ()> const& f, ContentTierManager const& m) {
        ResourcePackManager_construct(this, f, m);
    }

    void setStack(std::unique_ptr<ResourcePackStack>, ResourcePackStackType, bool);

    void onLanguageChanged();

};