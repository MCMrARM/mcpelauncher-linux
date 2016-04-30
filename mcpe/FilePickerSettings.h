#pragma once

#include <string>
#include <vector>
#include <functional>

struct FilePickerSettings {

    enum class PickerType {
        NONE, OPEN, SAVE
    };
    struct FileDescription {
        std::string ext, desc;
    };
    struct PickResult {
        std::string str;
    };

    char filler [0x20]; // 20
    std::function<void (PickResult)> pickedCallback; // 30
    std::vector<FileDescription> fileDescriptions; // 3c
    int filler3; // 40
    PickerType type; // 44
    std::string defaultFileName; // 48
    std::string pickerTitle; // 52

};