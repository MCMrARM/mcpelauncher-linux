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
#include "../mcpe/ImageData.h"
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
    region = "0xdeadbeef";
}

void LinuxAppPlatform::initVtable(void** base, int baseSize) {
    myVtable = (void**) ::operator new(baseSize * sizeof(void*));
    memcpy(&myVtable[0], &base[2], baseSize * sizeof(void*));

    myVtable[2] = (void*) &LinuxAppPlatform::getDataUrl;
    myVtable[4] = (void*) &LinuxAppPlatform::getImagePath;
    myVtable[5] = (void*) &LinuxAppPlatform::loadPNG;
    myVtable[12] = (void*) &LinuxAppPlatform::hideMousePointer;
    myVtable[13] = (void*) &LinuxAppPlatform::showMousePointer;
    myVtable[17] = (void*) &LinuxAppPlatform::swapBuffers;
    myVtable[19] = (void*) &LinuxAppPlatform::getSystemRegion;
    myVtable[20] = (void*) &LinuxAppPlatform::getGraphicsVendor;
    myVtable[21] = (void*) &LinuxAppPlatform::getGraphicsRenderer;
    myVtable[22] = (void*) &LinuxAppPlatform::getGraphicsVersion;
    myVtable[23] = (void*) &LinuxAppPlatform::getGraphicsExtensions;
    myVtable[24] = (void*) &LinuxAppPlatform::pickImage;
    myVtable[25] = (void*) &LinuxAppPlatform::pickFile;
    myVtable[26] = (void*) &LinuxAppPlatform::supportsFilePicking;
    myVtable[28] = (void*) &LinuxAppPlatform::getExternalStoragePath;
    myVtable[29] = (void*) &LinuxAppPlatform::getInternalStoragePath;
    myVtable[30] = (void*) &LinuxAppPlatform::getUserdataPath;
    myVtable[46] = (void*) &LinuxAppPlatform::readAssetFile;
    myVtable[59] = (void*) &LinuxAppPlatform::useCenteredGUI;
    myVtable[61] = (void*) &LinuxAppPlatform::getScreenType;
    myVtable[64] = (void*) &LinuxAppPlatform::getApplicationId;
    myVtable[65] = (void*) &LinuxAppPlatform::getAvailableMemory;
    myVtable[69] = (void*) &LinuxAppPlatform::getDeviceId;
    myVtable[70] = (void*) &LinuxAppPlatform::createUUID;
    myVtable[71] = (void*) &LinuxAppPlatform::isFirstSnoopLaunch;
    myVtable[72] = (void*) &LinuxAppPlatform::hasHardwareInformationChanged;
    myVtable[73] = (void*) &LinuxAppPlatform::isTablet;
    myVtable[82] = (void*) &LinuxAppPlatform::getEdition;
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

void LinuxAppPlatform::loadPNG(ImageData& imgData, const std::string& path, bool b) {
    std::cout << "loadPNG: " << path << " (from assets: " << b << ")\n";

    FILE* file = fopen(getImagePath(path, b).c_str(), "rb");
    if (file == NULL) {
        std::cout << "failed to open file\n";
        return;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        std::cout << "png_create_read_struct failed\n";
        abort();
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        std::cout << "png_create_info_struct failed\n";
        abort();
        return;
    }

    unsigned char header[8];
    if(fread(header, 8, 1, file) != 1) {
        std::cout << "failed to read png header\n";
        return;
    }

    if(png_sig_cmp(header, 0, 8)) {
        std::cout << "header is invalid\n";
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        std::cout << "failed to load png\n";
        abort();
        return;
    }

    png_init_io(png, file);
    png_set_sig_bytes(png, 8);

    png_read_info(png, info);

    imgData.w = (int) png_get_image_width(png, info);
    imgData.h = (int) png_get_image_height(png, info);
    imgData.format = TextureFormat::U8888;
    imgData.mipLevel = 0;

    png_byte bitDepth = png_get_bit_depth(png, info);
    png_byte colorType = png_get_color_type(png, info);
    switch(colorType){
        case PNG_COLOR_TYPE_PALETTE:
            png_set_palette_to_rgb(png);
            break;
        case PNG_COLOR_TYPE_RGB:
            if(png_get_valid(png, info, PNG_INFO_tRNS)) {
                png_set_tRNS_to_alpha(png);
            } else {
                png_set_filler(png, 0xff, PNG_FILLER_AFTER);
            }
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            break;
        case PNG_COLOR_TYPE_GRAY:
            if(bitDepth < 8) {
                png_set_expand_gray_1_2_4_to_8(png);
            }

        default:
            std::cout << "png: unsupported color type\n";
    }
    if(bitDepth == 16) {
        png_set_strip_16(png);
    }
    png_read_update_info(png, info);

    png_size_t rowBytes = png_get_rowbytes(png, info);

    std::vector<char> data(rowBytes * imgData.h);

    png_byte* rows[imgData.h];
    for (int i = 0; i < imgData.h; i++) {
        rows[i] = (png_byte*) &data[i * rowBytes];
    }
    png_read_image(png, rows);

    fclose(file);

    {
        std::string empty;
        ((void **) &imgData.data)[0] = ((void **) &empty)[0];
        imgData.data = " ";
    }
    imgData.data = std::string(&data[0], rowBytes * imgData.h);
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
    if (path.length() <= 0) {
        std::cout << "warn: readAssetFile with empty path!\n";
        return "-";
    }
    std::cout << "readAssetFile: " << path << "\n";
    std::ifstream ifs(std::string("assets/") + path);
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