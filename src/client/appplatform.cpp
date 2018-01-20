#include "appplatform.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <png.h>
#include <uuid/uuid.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include "../minecraft/ImagePickingCallback.h"
#include "../minecraft/FilePickerSettings.h"
#include "../common/log.h"
#include "../common/common.h"
#include "../common/path_helper.h"
#ifndef SERVER
#include "window.h"
#endif

extern "C" {
#include <hybris/dlfcn.h>
}

const char* LinuxAppPlatform::TAG = "AppPlatform";

void** LinuxAppPlatform::myVtable = nullptr;
bool enablePocketGuis = false;

LinuxAppPlatform::LinuxAppPlatform() : AppPlatform() {
    this->vtable = myVtable;
    dataDir = PathHelper::getPrimaryDataDirectory();
    assetsDir = PathHelper::findDataFile("assets/");
    tmpPath = PathHelper::getCacheDirectory();
    internalStorage = dataDir;
    externalStorage = dataDir;
    currentStorage = dataDir;
    userdata = dataDir;
    userdataPathForLevels = dataDir;
    region = "0xdeadbeef";
}

void LinuxAppPlatform::replaceVtableEntry(void* lib, void** vtable, const char* sym, void* nw) {
    void* sm = hybris_dlsym(lib, sym);
    for (int i = 0; ; i++) {
        if (vtable[i] == nullptr)
            break;
        if (vtable[i] == sm) {
            myVtable[i] = nw;
            return;
        }
    }
}

void LinuxAppPlatform::initVtable(void* lib) {
    void** vt = AppPlatform::myVtable;
    void** vta = &((void**) hybris_dlsym(lib, "_ZTV19AppPlatform_android"))[2];
    // get vtable size
    int size;
    for (size = 2; ; size++) {
        if (vt[size] == nullptr)
            break;
    }
    Log::trace("AppPlatform", "Vtable size = %i", size);

    myVtable = (void**) ::operator new(size * sizeof(void*));
    memcpy(&myVtable[0], &vt[2], (size - 2) * sizeof(void*));

    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android10getDataUrlEv", (void*) &LinuxAppPlatform::getDataUrl);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android14getUserDataUrlEv", (void*) &LinuxAppPlatform::getUserDataUrl);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14getPackagePathEv", (void*) &LinuxAppPlatform::getPackagePath);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16hideMousePointerEv", (void*) &LinuxAppPlatform::hideMousePointer);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16showMousePointerEv", (void*) &LinuxAppPlatform::showMousePointer);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android11swapBuffersEv", (void*) &LinuxAppPlatform::swapBuffers);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android15getSystemRegionEv", (void*) &LinuxAppPlatform::getSystemRegion);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android25getGraphicsTearingSupportEv", (void*) &LinuxAppPlatform::getGraphicsTearingSupport);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android9pickImageER20ImagePickingCallback", (void*) &LinuxAppPlatform::pickImage);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform8pickFileER18FilePickerSettings", (void*) &LinuxAppPlatform::pickFile);
    replaceVtableEntry(lib, vta, "_ZNK11AppPlatform19supportsFilePickingEv", (void*) &LinuxAppPlatform::supportsFilePicking);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android22getExternalStoragePathEv", (void*) &LinuxAppPlatform::getExternalStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android22getInternalStoragePathEv", (void*) &LinuxAppPlatform::getInternalStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android21getCurrentStoragePathEv", (void*) &LinuxAppPlatform::getCurrentStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android15getUserdataPathEv", (void*) &LinuxAppPlatform::getUserdataPath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android24getUserdataPathForLevelsEv", (void*) &LinuxAppPlatform::getUserdataPathForLevels);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform20getAssetFileFullPathERKSs", (void*) &LinuxAppPlatform::getAssetFileFullPath);
    replaceVtableEntry(lib, vta, "_ZNK11AppPlatform14useCenteredGUIEv", (void*) &LinuxAppPlatform::useCenteredGUI);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android16getApplicationIdEv", (void*) &LinuxAppPlatform::getApplicationId);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android25_updateUsedMemorySnapshotEv", (void*) &LinuxAppPlatform::_updateUsedMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android30_updateAvailableMemorySnapshotEv", (void*) &LinuxAppPlatform::_updateAvailableMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android26_updateTotalMemorySnapshotEv", (void*) &LinuxAppPlatform::_updateTotalMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android11getDeviceIdEv", (void*) &LinuxAppPlatform::getDeviceId);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18isFirstSnoopLaunchEv", (void*) &LinuxAppPlatform::isFirstSnoopLaunch);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android29hasHardwareInformationChangedEv", (void*) &LinuxAppPlatform::hasHardwareInformationChanged);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android8isTabletEv", (void*) &LinuxAppPlatform::isTablet);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform17setFullscreenModeE14FullscreenMode", (void*) &LinuxAppPlatform::setFullscreenMode);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android10getEditionEv", (void*) &LinuxAppPlatform::getEdition);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android31calculateAvailableDiskFreeSpaceERKSs", (void*) &LinuxAppPlatform::calculateAvailableDiskFreeSpace);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android25getPlatformUIScalingRulesEv", (void*) &LinuxAppPlatform::getPlatformUIScalingRules);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android19getPlatformTempPathEv", (void*) &LinuxAppPlatform::getPlatformTempPath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14createDeviceIDEv", (void*) &LinuxAppPlatform::createDeviceID_old);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14createDeviceIDERSs", (void*) &LinuxAppPlatform::createDeviceID);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18queueForMainThreadESt8functionIFvvEE", (void*) &LinuxAppPlatform::queueForMainThread);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android35getMultiplayerServiceListToRegisterEv", (void*) &LinuxAppPlatform::getMultiplayerServiceListToRegister);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16allowSplitScreenEv", (void*) &LinuxAppPlatform::allowSplitScreen);
}

void LinuxAppPlatform::hideMousePointer() {
#ifndef SERVER
    window->setCursorDisabled(true);
#endif
}
void LinuxAppPlatform::showMousePointer() {
#ifndef SERVER
    window->setCursorDisabled(false);
#endif
}

std::string LinuxAppPlatform::_pickFile(std::string commandLine) {
    Log::trace(TAG, "Launching file picker with args: %s", commandLine.c_str());
    char file[1024];
    FILE *f = popen(commandLine.c_str(), "r");
    if (fgets(file, 1024, f) == nullptr) {
        Log::trace(TAG, "No file selected");
        return "";
    }
    file[strlen(file) - 1] = '\0';
    Log::trace(TAG, "Selected file: %s", file);
    return std::string(file);
}

void LinuxAppPlatform::pickImage(ImagePickingCallback &callback) {
    Log::trace(TAG, "pickImage");
    std::string file = _pickFile("zenity --file-selection --title 'Select image' --file-filter '*.png'");
    if (file.empty()) {
        callback.onImagePickingCanceled();
    } else {
        callback.onImagePickingSuccess(file);
    }
}

std::string replaceAll(std::string s, std::string a, std::string b) {
    while (true) {
        size_t p = s.find(a);
        if (p == std::string::npos)
            break;
        s.replace(p, a.length(), b);
    }
    return s;
}

void LinuxAppPlatform::pickFile(FilePickerSettings &settings) {
    std::cout << "pickFile\n";
    std::cout << "- title: " << settings.pickerTitle << "\n";
    std::cout << "- type: " << (int) settings.type << "\n";
    std::cout << "- file descriptions:\n";
    for (FilePickerSettings::FileDescription &d : settings.fileDescriptions) {
        std::cout << " - " << d.ext << " " << d.desc << "\n";
    }
    std::stringstream ss;
    ss << "zenity --file-selection --title '" << replaceAll(settings.pickerTitle.std(), "'", "\\'") << "'";
    if (settings.type == FilePickerSettings::PickerType::SAVE)
        ss << " --save";
    if (settings.fileDescriptions.size() > 0) {
        ss << " --file-filter '";
        bool first = true;
        for (FilePickerSettings::FileDescription &d : settings.fileDescriptions) {
            if (first)
                first = false;
            else
                ss << "|";
            ss << "*." << d.ext;
        }
        ss << "'";
    }
    std::string file = _pickFile(ss.str());
    if (file.empty()) {
        settings.cancelCallback(settings);
    } else {
        settings.pickedCallback(settings, file);
    }
}

void LinuxAppPlatform::setFullscreenMode(int mode) {
    Log::trace(TAG, "Changing fullscreen mode: %i", mode);
#ifndef SERVER
    window->setFullscreen(mode != 0);
#endif
}

long long LinuxAppPlatform::calculateAvailableDiskFreeSpace() {
    struct statvfs buf;
    statvfs(dataDir.c_str(), &buf);
    return (long long int) buf.f_bsize * buf.f_bfree;
}

void LinuxAppPlatform::_updateUsedMemorySnapshot() {
    FILE* file = fopen("/proc/self/statm", "r");
    if (file == nullptr)
        return;
    int pageSize = getpagesize();
    long long pageCount = 0L;
    fscanf(file, "%lld", &pageCount);
    fclose(file);
    usedMemory = pageCount * pageSize;
}

void LinuxAppPlatform::_updateAvailableMemorySnapshot() {
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long total = memInfo.freeram;
    total += memInfo.freeswap;
    total *= memInfo.mem_unit;
    availableMemory = total;
}

void LinuxAppPlatform::_updateTotalMemorySnapshot() {
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long total = memInfo.totalram;
    total += memInfo.totalswap;
    total *= memInfo.mem_unit;
    totalMemory = total;
}