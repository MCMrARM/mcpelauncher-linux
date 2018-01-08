#pragma once

#include <string>

class FilePathManager {

public:

    char filler[0x20];

    static void (*FilePathManager_construct)(FilePathManager*, mcpe::string, bool);

    FilePathManager(mcpe::string str, bool b) {
        FilePathManager_construct(this, std::move(str), b);
    }


};