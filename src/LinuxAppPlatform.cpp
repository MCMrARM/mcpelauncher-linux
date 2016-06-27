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

extern "C" {
#include <eglut.h>
}

void** LinuxAppPlatform::myVtable = nullptr;
bool LinuxAppPlatform::mousePointerHidden = false;
bool enablePocketGuis = false;

LinuxAppPlatform::LinuxAppPlatform() : AppPlatform() {
    this->vtable = myVtable;
    internalStorage = "data/private/";
    externalStorage = "data/public/";
    userdata = "data/user/";
    userdataPathForLevels = "data/user/";
    region = "0xdeadbeef";
    tmpPath = "tmp/";
}

void LinuxAppPlatform::initVtable(void** base, int baseSize) {
    void** myVtable = (void**) ::operator new(baseSize * sizeof(void*));
    memcpy(&myVtable[0], &base[2], baseSize * sizeof(void*));

    myVtable[2] = (void*) &LinuxAppPlatform::getDataUrl;
    myVtable[4] = (void*) &LinuxAppPlatform::getPackagePath;
    myVtable[13] = (void*) &LinuxAppPlatform::hideMousePointer;
    myVtable[14] = (void*) &LinuxAppPlatform::showMousePointer;
    myVtable[18] = (void*) &LinuxAppPlatform::swapBuffers;
    myVtable[20] = (void*) &LinuxAppPlatform::getSystemRegion;
    myVtable[21] = (void*) &LinuxAppPlatform::getGraphicsVendor;
    myVtable[22] = (void*) &LinuxAppPlatform::getGraphicsRenderer;
    myVtable[23] = (void*) &LinuxAppPlatform::getGraphicsVersion;
    myVtable[24] = (void*) &LinuxAppPlatform::getGraphicsExtensions;
    myVtable[25] = (void*) &LinuxAppPlatform::pickImage;
    myVtable[26] = (void*) &LinuxAppPlatform::pickFile;
    myVtable[27] = (void*) &LinuxAppPlatform::supportsFilePicking;
    myVtable[33] = (void*) &LinuxAppPlatform::getExternalStoragePath;
    myVtable[34] = (void*) &LinuxAppPlatform::getInternalStoragePath;
    myVtable[35] = (void*) &LinuxAppPlatform::getUserdataPath;
    myVtable[36] = (void*) &LinuxAppPlatform::getUserdataPathForLevels;
    myVtable[53] = (void*) &LinuxAppPlatform::getAssetFileFullPath;
    myVtable[54] = (void*) &LinuxAppPlatform::readAssetFile;
    myVtable[68] = (void*) &LinuxAppPlatform::useCenteredGUI;
    myVtable[74] = (void*) &LinuxAppPlatform::getApplicationId;
    myVtable[75] = (void*) &LinuxAppPlatform::getAvailableMemory;
    myVtable[80] = (void*) &LinuxAppPlatform::getDeviceId;
    myVtable[81] = (void*) &LinuxAppPlatform::createUUID;
    myVtable[82] = (void*) &LinuxAppPlatform::isFirstSnoopLaunch;
    myVtable[83] = (void*) &LinuxAppPlatform::hasHardwareInformationChanged;
    myVtable[84] = (void*) &LinuxAppPlatform::isTablet;
    myVtable[93] = (void*) &LinuxAppPlatform::getEdition;
    myVtable[100] = (void*) &LinuxAppPlatform::getPlatformTempPath;
    LinuxAppPlatform::myVtable = myVtable;
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

std::string LinuxAppPlatform::readAssetFile(const std::string& path) {
    if (path.length() <= 0 || path == "assets/") {
        std::cout << "warn: readAssetFile with empty path!\n";
        return "-";
    }
    std::cout << "readAssetFile: " << path << "\n";
    std::ifstream ifs(path);
    if (!ifs) {
        std::cout << "readAssetFile failed\n";
        return "-";
    }
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       (std::istreambuf_iterator<char>()));
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