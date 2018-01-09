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

    static void (*SkinPackKeyProvider_construct)(SkinPackKeyProvider*);

    SkinPackKeyProvider() {
        SkinPackKeyProvider_construct(this);
    }


};

class PackManifestFactory {

public:

    char filler[4];

    static void (*PackManifestFactory_construct)(PackManifestFactory*, IPackTelemetry&);

    PackManifestFactory(IPackTelemetry& ev) {
        PackManifestFactory_construct(this, ev);
    }

};

class PackSourceFactory {

public:

    char filler[0x100];

    static void (*PackSourceFactory_construct)(PackSourceFactory*, Options *);

    PackSourceFactory(Options* o) {
        PackSourceFactory_construct(this, o);
    }

};

class ResourcePackRepository {

public:

    char filler[0x28];
    ResourcePack* vanillaPack;
    char filler2[0x100];

    static void (*ResourcePackRepository_construct)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&, IContentAccessibilityProvider&, FilePathManager*, PackSourceFactory&);

    ResourcePackRepository(MinecraftEventing& ev, PackManifestFactory& fact, IContentAccessibilityProvider& ap, FilePathManager* pm, PackSourceFactory& ps) {
        ResourcePackRepository_construct(this, ev, fact, ap, pm, ps);
    }

};

struct ContentTierManager {

public:

    static void (*ContentTierManager_construct)(ContentTierManager*);

    int filler;

    ContentTierManager() {
        ContentTierManager_construct(this);
    }

};

class ResourcePackManager {

public:

    char filler[0x100];

    static void (*ResourcePackManager_construct)(ResourcePackManager*, std::function<mcpe::string ()> const&, ContentTierManager const&);
    static void (*ResourcePackManager_setStack)(ResourcePackManager*, std::unique_ptr<ResourcePackStack>, ResourcePackStackType, bool);
    static void (*ResourcePackManager_onLanguageChanged)(ResourcePackManager*);

    ResourcePackManager(std::function<std::string ()> const& f, ContentTierManager const& m) {
        ResourcePackManager_construct(this, f, m);
    }

    void setStack(std::unique_ptr<ResourcePackStack> s, ResourcePackStackType t, bool b) {
        ResourcePackManager_setStack(this, std::move(s), t, b);
    }

    void onLanguageChanged() {
        ResourcePackManager_onLanguageChanged(this);
    }

};