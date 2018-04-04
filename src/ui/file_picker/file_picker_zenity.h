#pragma once

#include "file_picker.h"

class ZenityFilePicker : public FilePicker {

private:
    std::string title;
    Mode mode;
    std::vector<std::string> patterns;
    std::string pickedFile;

    static const std::string EXECUTABLE_PATH;

    std::vector<std::string> buildCommandLine();

    static std::vector<const char*> convertToC(std::vector<std::string> const& v);

public:
    void setTitle(std::string const& title) override {
        this->title = title;
    }

    void setMode(Mode mode) {
        this->mode = mode;
    }

    void setFileNameFilters(std::vector<std::string> const& patterns) override {
        this->patterns = patterns;
    }

    bool show() override;

    std::string getPickedFile() const override {
        return pickedFile;
    }

};