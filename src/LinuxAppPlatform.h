#pragma once

#include <string>
#include <unistd.h>
#include <sys/param.h>
#include "../mcpe/gl.h"
#include "../mcpe/AppPlatform.h"
#include "../mcpe/ImagePickingCallback.h"

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

    std::string region;
    std::string internalStorage, externalStorage, userdata, userdataPathForLevels, tmpPath;

    LinuxAppPlatform();

    std::string getDataUrl() { // this is used only for sounds
        printf("get data url: assets/\n");
        return "assets/";
    }
    std::string getUserDataUrl() { // this is used only for sounds
        printf("get user data url: data/user/\n");
        return "data/user/";
    }

    std::string getPackagePath() {
        return "assets/";
    }

    void hideMousePointer();
    void showMousePointer();

    void swapBuffers() {
        //printf("swap buffers\n");
    }
    std::string const& getSystemRegion() {
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
    std::string& getExternalStoragePath() {
        printf("external storage path = %s\n", externalStorage.c_str());
        return externalStorage;
    }
    std::string& getInternalStoragePath() {
        printf("internal storage path = %s\n", internalStorage.c_str());
        return internalStorage;
    }
    std::string& getUserdataPath() {
        printf("userdata path = %s\n", userdata.c_str());
        return userdata;
    }
    std::string& getUserdataPathForLevels() {
        printf("userdata path for levels = %s\n", userdata.c_str());
        return userdataPathForLevels;
    }
    std::string getAssetFileFullPath(std::string const& s) {
        printf("get assert full path: %s\n", s.c_str());
        return "assets/" + s;
    }
    int getScreenType() {
        if (enablePocketGuis)
            return 1;
        return 0; // Win 10 Ed. GUIs
    }
    bool useCenteredGUI() {
        return (enablePocketGuis ? false : true);
    }
    std::string getApplicationId() {
        printf("application id = com.mojang.minecraftpe\n");
        return "com.mojang.minecraftpe";
    }
    std::string getDeviceId() {
        printf("device id = linux\n");
        return "linux";
    }
    std::string createUUID();
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
    std::string getEdition() {
        if (enablePocketGuis)
            return "pocket";
        return "win10";
    }
    int getPlatformUIScalingRules() {
        return 2;
    }
    long long getAvailableMemory();

    long long calculateAvailableDiskFreeSpace() {
        return 100000000L;
    }

    std::string &getPlatformTempPath() {
        return tmpPath;
    }

};
