#pragma once

#include <string>

class FilePathManager {

public:

    char filler[0x20];

    FilePathManager(mcpe::string, bool);

    mcpe::string getRootPath() const;

    mcpe::string getUserDataPath() const;

    mcpe::string getWorldsPath() const;

    void setPackagePath(mcpe::string);
    mcpe::string getPackagePath() const;

    void setSettingsPath(mcpe::string);
    mcpe::string getSettingsPath() const;


};