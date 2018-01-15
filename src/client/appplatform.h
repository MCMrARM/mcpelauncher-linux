#pragma once

#include <string>
#include <functional>
#include <unistd.h>
#include <sys/param.h>
#include <vector>
#include <mutex>
#include <memory>
#include "../common/log.h"
#include "../common/path_helper.h"
#include "../minecraft/gl.h"
#include "../minecraft/AppPlatform.h"
#include "../minecraft/ImagePickingCallback.h"
#include "../minecraft/MultiplayerService.h"

class ImageData;
class ImagePickingCallback;
class FilePickerSettings;
class GameWindow;

extern bool enablePocketGuis;

class LinuxAppPlatform : public AppPlatform {

private:
    static const char* TAG;

    static std::string _pickFile(std::string commandLine);

    static void replaceVtableEntry(void* lib, void** vtable, const char* sym, void* nw);

#ifndef SERVER
    GameWindow* window;
#endif

public:
    static void** myVtable;
    static void initVtable(void* lib);

    mcpe::string region;
    mcpe::string internalStorage, externalStorage, currentStorage, userdata, userdataPathForLevels, tmpPath;

    std::string assetsDir, dataDir;

    std::vector<std::function<void ()>> runOnMainThreadQueue;
    std::mutex runOnMainThreadMutex;

    LinuxAppPlatform();

#ifndef SERVER
    void setWindow(GameWindow* window) { this->window = window; }
#endif

    mcpe::string getDataUrl() { // this is used only for sounds
        Log::trace(TAG, "getDataUrl: %s", assetsDir.c_str());
        return assetsDir;
    }
    mcpe::string getUserDataUrl() {
        Log::trace(TAG, "getUserDataUrl: %s", dataDir.c_str());
        return dataDir;
    }

    mcpe::string getPackagePath() {
        return assetsDir;
    }

    void hideMousePointer();
    void showMousePointer();

    void swapBuffers() {
        //printf("swap buffers\n");
    }
    mcpe::string const& getSystemRegion() {
        Log::trace(TAG, "getSystemRegion: %s", region.c_str());
        return region;
    }

    bool getGraphicsTearingSupport() {
        return false;
    }

    void pickImage(ImagePickingCallback& callback);
    void pickFile(FilePickerSettings& callback);
    bool supportsFilePicking() {
        return true;
    }
    mcpe::string& getExternalStoragePath() {
        Log::trace(TAG, "getExternalStoragePath: %s", externalStorage.c_str());
        return externalStorage;
    }
    mcpe::string& getInternalStoragePath() {
        Log::trace(TAG, "getInternalStoragePath: %s", internalStorage.c_str());
        return internalStorage;
    }
    mcpe::string& getCurrentStoragePath() {
        Log::trace(TAG, "getCurrentStoragePath: %s", currentStorage.c_str());
        return currentStorage;
    }
    mcpe::string& getUserdataPath() {
        Log::trace(TAG, "getUserdataPath: %s", userdata.c_str());
        return userdata;
    }
    mcpe::string& getUserdataPathForLevels() {
        Log::trace(TAG, "getUserdataPathForLevels: %s", userdataPathForLevels.c_str());
        return userdataPathForLevels;
    }
    mcpe::string getAssetFileFullPath(mcpe::string const& s) {
        Log::trace(TAG, "getAssetFileFullPath: %s", s.c_str());
        return mcpe::string(assetsDir) + s;
    }
    int getScreenType() {
        if (enablePocketGuis)
            return 1;
        return 0; // Win 10 Ed. GUIs
    }
    bool useCenteredGUI() {
        return (enablePocketGuis ? false : true);
    }
    mcpe::string getApplicationId() {
        Log::trace(TAG, "getApplicationId: com.mojang.minecraftpe");
        return "com.mojang.minecraftpe";
    }
    mcpe::string getDeviceId() {
        Log::trace(TAG, "getDeviceId: linux");
        return "linux";
    }
    mcpe::string createUUID();
    bool isFirstSnoopLaunch() {
        Log::trace(TAG, "isFirstSnoopLaunch: true");
        return true;
    }
    bool hasHardwareInformationChanged() {
        Log::trace(TAG, "hasHardwareInformationChanged: false");
        return false;
    }
    bool isTablet() {
        Log::trace(TAG, "isTablet: true");
        return true;
    }
    void setFullscreenMode(int mode);
    mcpe::string getEdition() {
        if (enablePocketGuis)
            return "pocket";
        return "win10";
    }
    int getPlatformUIScalingRules() {
        return enablePocketGuis ? 2 : 0;
    }
    void _updateUsedMemorySnapshot();
    void _updateAvailableMemorySnapshot();
    void _updateTotalMemorySnapshot();

    long long calculateAvailableDiskFreeSpace();

    mcpe::string &getPlatformTempPath() {
        return tmpPath;
    }

    mcpe::string createDeviceID_old() {
        return "linux";
    }

    mcpe::string createDeviceID(std::string const& c) {
        Log::trace(TAG, "createDeviceID: %s", c.c_str());
        return "linux";
    }

    std::vector<std::shared_ptr<Social::MultiplayerService>> getMultiplayerServiceListToRegister() {
        std::vector<std::shared_ptr<Social::MultiplayerService>> ret;
        ret.push_back(std::shared_ptr<Social::MultiplayerService>(new Social::MultiplayerXBL()));
        return ret;
    }

    bool allowSplitScreen() {
        return true;
    }

    void queueForMainThread(std::function<void ()> f) {
        runOnMainThreadMutex.lock();
        runOnMainThreadQueue.push_back(f);
        runOnMainThreadMutex.unlock();
    }
    void runMainThreadTasks() {
        runOnMainThreadMutex.lock();
        auto queue = std::move(runOnMainThreadQueue);
        runOnMainThreadMutex.unlock();
        for (auto const& func : queue)
            func();
    }

};
