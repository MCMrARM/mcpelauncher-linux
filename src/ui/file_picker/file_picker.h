#pragma once

#include <string>
#include <vector>

class FilePicker {

public:
    enum class Mode {
        OPEN, SAVE
    };

    virtual ~FilePicker() {}

    virtual void setTitle(std::string const& title) = 0;

    virtual void setMode(Mode mode) = 0;

    virtual void setFileNameFilters(std::vector<std::string> const& patterns) = 0;

    virtual bool show() = 0;

    virtual std::string getPickedFile() const = 0;

};