#pragma once

#include <memory>
#include "file_picker.h"

class FilePickerFactory {

public:

    static std::unique_ptr<FilePicker> createFilePicker();

};