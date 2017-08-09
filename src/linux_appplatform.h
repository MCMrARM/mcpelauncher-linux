#pragma once

#include <string>
#include <functional>
#include <unistd.h>
#include <sys/param.h>
#include <vector>
#include <mutex>
#include "minecraft/gl.h"
#include "minecraft/AppPlatform.h"
#include "minecraft/ImagePickingCallback.h"

class ImageData;
class ImagePickingCallback;
class FilePickerSettings;

extern bool enablePocketGuis;
extern bool moveMouseToCenter;

class LinuxAppPlatform : public AppPlatform {

private:
    static std::string _pickFile(std::string commandLine);

    static void replaceVtableEntry(void* lib, void** vtable, const char* sym, void* nw);

public:
    static void** myVtable;
    static void initVtable(void* lib);

    static bool mousePointerHidden;

    mcpe::string region;
    mcpe::string internalStorage, externalStorage, currentStorage, userdata, userdataPathForLevels, tmpPath;

    std::vector<std::function<void ()>> runOnMainThreadQueue;
    std::mutex runOnMainThreadMutex;

    LinuxAppPlatform();

    mcpe::string getDataUrl() { // this is used only for sounds
        printf("get data url: assets/\n");
        return "assets/";
    }
    mcpe::string getUserDataUrl() { // this is used only for sounds
        printf("get user data url: data/user/\n");
        return "data/user/";
    }

    mcpe::string getPackagePath() {
        return "assets/";
    }

    void hideMousePointer();
    void showMousePointer();

    void swapBuffers() {
        //printf("swap buffers\n");
    }
    mcpe::string const& getSystemRegion() {
        printf("get system region: %s\n", region.c_str());
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
        printf("external storage path = %s\n", externalStorage.c_str());
        return externalStorage;
    }
    mcpe::string& getInternalStoragePath() {
        printf("internal storage path = %s\n", internalStorage.c_str());
        return internalStorage;
    }
    mcpe::string& getCurrentStoragePath() {
        printf("current storage path = %s\n", currentStorage.c_str());
        return currentStorage;
    }
    mcpe::string& getUserdataPath() {
        printf("userdata path = %s\n", userdata.c_str());
        return userdata;
    }
    mcpe::string& getUserdataPathForLevels() {
        printf("userdata path for levels = %s\n", userdata.c_str());
        return userdataPathForLevels;
    }
    mcpe::string getAssetFileFullPath(mcpe::string const& s) {
        printf("get assert full path: %s\n", s.c_str());
        return mcpe::string("assets/") + s;
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
        printf("application id = com.mojang.minecraftpe\n");
        return "com.mojang.minecraftpe";
    }
    mcpe::string getDeviceId() {
        printf("device id = linux\n");
        return "linux";
    }
    mcpe::string createUUID();
    bool isFirstSnoopLaunch() {
        printf("is first snoop launch = true\n");
        return true;
    }
    bool hasHardwareInformationChanged() {
        printf("has hardware information change = false\n");
        return false;
    }
    bool isTablet() {
        printf("is tablet = true\n");
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
    long long getAvailableMemory();

    long long calculateAvailableDiskFreeSpace();

    mcpe::string &getPlatformTempPath() {
        return tmpPath;
    }

    mcpe::string createDeviceID() {
        return "linux";
    }

    void queueForMainThread(std::function<void ()> f) {
        runOnMainThreadMutex.lock();
        runOnMainThreadQueue.push_back(f);
        runOnMainThreadMutex.unlock();
    }

};
