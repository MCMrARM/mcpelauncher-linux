#pragma once

#include "std/string.h"
#include <vector>
#include <functional>

struct FilePickerSettings {

    enum class PickerType {
        NONE, OPEN, SAVE
    };
    struct FileDescription {
        mcpe::string ext, desc;
    };

    char filler [0x10]; // 10
    std::function<void (FilePickerSettings&)> cancelCallback; // 20
    std::function<void (FilePickerSettings&, mcpe::string)> pickedCallback; // 30
    std::vector<FileDescription> fileDescriptions; // 3c
    int filler3; // 40
    PickerType type; // 44
    mcpe::string defaultFileName; // 48
    mcpe::string pickerTitle; // 52

};