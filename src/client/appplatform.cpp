#include "appplatform.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#ifndef __APPLE__
#include <sys/sysinfo.h>
#endif
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
#include "../ui/game_window/window.h"
#include "../ui/file_picker/file_picker_factory.h"
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

    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android10getDataUrlEv", &LinuxAppPlatform::getDataUrl);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android14getUserDataUrlEv", &LinuxAppPlatform::getUserDataUrl);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14getPackagePathEv", &LinuxAppPlatform::getPackagePath);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16hideMousePointerEv", &LinuxAppPlatform::hideMousePointer);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16showMousePointerEv", &LinuxAppPlatform::showMousePointer);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android11swapBuffersEv", &LinuxAppPlatform::swapBuffers);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android15getSystemRegionEv", &LinuxAppPlatform::getSystemRegion);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android25getGraphicsTearingSupportEv", &LinuxAppPlatform::getGraphicsTearingSupport);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android9pickImageER20ImagePickingCallback", &LinuxAppPlatform::pickImage);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform8pickFileER18FilePickerSettings", &LinuxAppPlatform::pickFile);
    replaceVtableEntry(lib, vta, "_ZNK11AppPlatform19supportsFilePickingEv", &LinuxAppPlatform::supportsFilePicking);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android22getExternalStoragePathEv", &LinuxAppPlatform::getExternalStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android22getInternalStoragePathEv", &LinuxAppPlatform::getInternalStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android21getCurrentStoragePathEv", &LinuxAppPlatform::getCurrentStoragePath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android15getUserdataPathEv", &LinuxAppPlatform::getUserdataPath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android24getUserdataPathForLevelsEv", &LinuxAppPlatform::getUserdataPathForLevels);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform20getAssetFileFullPathERKSs", &LinuxAppPlatform::getAssetFileFullPath);
    replaceVtableEntry(lib, vta, "_ZNK11AppPlatform14useCenteredGUIEv", &LinuxAppPlatform::useCenteredGUI);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android16getApplicationIdEv", &LinuxAppPlatform::getApplicationId);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android25_updateUsedMemorySnapshotEv", &LinuxAppPlatform::_updateUsedMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android30_updateAvailableMemorySnapshotEv", &LinuxAppPlatform::_updateAvailableMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android26_updateTotalMemorySnapshotEv", &LinuxAppPlatform::_updateTotalMemorySnapshot);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android11getDeviceIdEv", &LinuxAppPlatform::getDeviceId);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18isFirstSnoopLaunchEv", &LinuxAppPlatform::isFirstSnoopLaunch);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android29hasHardwareInformationChangedEv", &LinuxAppPlatform::hasHardwareInformationChanged);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android8isTabletEv", &LinuxAppPlatform::isTablet);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform17setFullscreenModeE14FullscreenMode", &LinuxAppPlatform::setFullscreenMode);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android10getEditionEv", &LinuxAppPlatform::getEdition);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android16getBuildPlatformEv", &LinuxAppPlatform::getBuildPlatform);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android31calculateAvailableDiskFreeSpaceERKSs", &LinuxAppPlatform::calculateAvailableDiskFreeSpace);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android25getPlatformUIScalingRulesEv", &LinuxAppPlatform::getPlatformUIScalingRules);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android19getPlatformTempPathEv", &LinuxAppPlatform::getPlatformTempPath);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14createDeviceIDEv", &LinuxAppPlatform::createDeviceID_old);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android14createDeviceIDERSs", &LinuxAppPlatform::createDeviceID);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18queueForMainThreadESt8functionIFvvEE", &LinuxAppPlatform::queueForMainThread);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android35getMultiplayerServiceListToRegisterEv", &LinuxAppPlatform::getMultiplayerServiceListToRegister);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform16allowSplitScreenEv", &LinuxAppPlatform::allowSplitScreen);
}


#ifndef SERVER

void LinuxAppPlatform::hideMousePointer() {
    window->setCursorDisabled(true);
}
void LinuxAppPlatform::showMousePointer() {
    window->setCursorDisabled(false);
}

void LinuxAppPlatform::pickImage(ImagePickingCallback &callback) {
    Log::trace(TAG, "pickImage");
    auto picker = FilePickerFactory::createFilePicker();
    picker->setTitle("Select image");
    picker->setFileNameFilters({ "*.png" });
    if (picker->show())
        callback.onImagePickingSuccess(picker->getPickedFile());
    else
        callback.onImagePickingCanceled();
}

void LinuxAppPlatform::pickFile(FilePickerSettings &settings) {
    std::cout << "pickFile\n";
    std::cout << "- title: " << settings.pickerTitle << "\n";
    std::cout << "- type: " << (int) settings.type << "\n";
    std::cout << "- file descriptions:\n";
    for (FilePickerSettings::FileDescription &d : settings.fileDescriptions) {
        std::cout << " - " << d.ext << " " << d.desc << "\n";
    }

    auto picker = FilePickerFactory::createFilePicker();
    picker->setTitle(settings.pickerTitle.std());
    if (settings.type == FilePickerSettings::PickerType::SAVE)
        picker->setMode(FilePicker::Mode::SAVE);
    std::vector<std::string> filters;
    for (auto const& filter : settings.fileDescriptions)
        filters.push_back(std::string("*.") + filter.ext.std());
    picker->setFileNameFilters(filters);
    if (picker->show())
        settings.pickedCallback(settings, picker->getPickedFile());
    else
        settings.cancelCallback(settings);
}

void LinuxAppPlatform::setFullscreenMode(int mode) {
    Log::trace(TAG, "Changing fullscreen mode: %i", mode);
    window->setFullscreen(mode != 0);
}

#else

// Stubs

void LinuxAppPlatform::hideMousePointer() {
}

void LinuxAppPlatform::showMousePointer() {
}

void LinuxAppPlatform::pickImage(ImagePickingCallback &callback) {
}

void LinuxAppPlatform::pickFile(FilePickerSettings &settings) {
}

void LinuxAppPlatform::setFullscreenMode(int mode) {
}

#endif

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
#ifndef __APPLE__
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long total = memInfo.freeram;
    total += memInfo.freeswap;
    total *= memInfo.mem_unit;
    availableMemory = total;
#endif
}

void LinuxAppPlatform::_updateTotalMemorySnapshot() {
#ifndef __APPLE__
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long total = memInfo.totalram;
    total += memInfo.totalswap;
    total *= memInfo.mem_unit;
    totalMemory = total;
#endif
}
