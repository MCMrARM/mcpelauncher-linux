#include "LinuxAppPlatform.h"
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
#include "../mcpe/ImagePickingCallback.h"
#include "../mcpe/FilePickerSettings.h"
#include "../hybris/src/jb/linker.h"

extern "C" {
#include <eglut.h>
#include "../hybris/include/hybris/dlfcn.h"
}

void** LinuxAppPlatform::myVtable = nullptr;
bool LinuxAppPlatform::mousePointerHidden = false;
bool enablePocketGuis = false;

LinuxAppPlatform::LinuxAppPlatform() : AppPlatform() {
    this->vtable = myVtable;
    internalStorage = "data/private/";
    externalStorage = "data/public/";
    currentStorage = "data/current/";
    userdata = "data/user/";
    userdataPathForLevels = "data/user/";
    region = "0xdeadbeef";
    tmpPath = "tmp/";
}

#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

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
    printf("AppPlatform size = %i\n", size);

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
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18getAvailableMemoryEv", (void*) &LinuxAppPlatform::getAvailableMemory);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android11getDeviceIdEv", (void*) &LinuxAppPlatform::getDeviceId);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android10createUUIDEv", (void*) &LinuxAppPlatform::createUUID);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android18isFirstSnoopLaunchEv", (void*) &LinuxAppPlatform::isFirstSnoopLaunch);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android29hasHardwareInformationChangedEv", (void*) &LinuxAppPlatform::hasHardwareInformationChanged);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android8isTabletEv", (void*) &LinuxAppPlatform::isTablet);
    replaceVtableEntry(lib, vta, "_ZN11AppPlatform17setFullscreenModeE14FullscreenMode", (void*) &LinuxAppPlatform::setFullscreenMode);
    replaceVtableEntry(lib, vta, "_ZNK11AppPlatform10getEditionEv", (void*) &LinuxAppPlatform::getEdition);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android31calculateAvailableDiskFreeSpaceERKSs", (void*) &LinuxAppPlatform::calculateAvailableDiskFreeSpace);
    replaceVtableEntry(lib, vta, "_ZNK19AppPlatform_android25getPlatformUIScalingRulesEv", (void*) &LinuxAppPlatform::getPlatformUIScalingRules);
    replaceVtableEntry(lib, vta, "_ZN19AppPlatform_android19getPlatformTempPathEv", (void*) &LinuxAppPlatform::getPlatformTempPath);
}

void LinuxAppPlatform::hideMousePointer() {
    mousePointerHidden = true;
    moveMouseToCenter = true;
    eglutSetMousePointerVisiblity(EGLUT_POINTER_INVISIBLE);
}
void LinuxAppPlatform::showMousePointer() {
    mousePointerHidden = false;
    eglutSetMousePointerVisiblity(EGLUT_POINTER_VISIBLE);
}

std::string LinuxAppPlatform::_pickFile(std::string commandLine) {
    std::cout << "Launching file picker with args: " << commandLine << "\n";
    char file[1024];
    FILE *f = popen(commandLine.c_str(), "r");
    if (fgets(file, 1024, f) == nullptr) {
        std::cout << "No file selected\n";
        return "";
    }
    file[strlen(file) - 1] = '\0';
    std::cout << "Selected file: " << file << "\n";
    return std::string(file);
}

void LinuxAppPlatform::pickImage(ImagePickingCallback &callback) {
    std::cout << "pickImage\n";
    std::string file = _pickFile("zenity --file-selection --title 'Select image' --file-filter *.png");
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
    ss << "zenity --file-selection --title '" << replaceAll(settings.pickerTitle, "'", "\\'") << "'";
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
    settings.pickedCallback(settings, file);
}

void LinuxAppPlatform::setFullscreenMode(int mode) {
    std::cout << "set fullscreen mode: " << mode << "\n";
    int newMode = mode == 1 ? EGLUT_FULLSCREEN : EGLUT_WINDOWED;
    if (eglutGet(EGLUT_FULLSCREEN_MODE) != newMode)
        eglutToggleFullscreen();
}

std::string LinuxAppPlatform::createUUID() {
    srand(time(NULL));

    uuid_t id;
    uuid_generate(id);
    char out [256];
    uuid_unparse(id, out);
    printf("uuid: %s\n", out);
    return std::string(out);
}

long long LinuxAppPlatform::getAvailableMemory() {
    struct sysinfo memInfo;
    sysinfo (&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    return totalVirtualMem;
}