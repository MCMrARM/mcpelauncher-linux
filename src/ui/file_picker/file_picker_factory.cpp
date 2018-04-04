#include "file_picker_factory.h"

#include <stdexcept>

#ifdef USE_ZENITY
#include "file_picker_zenity.h"
std::unique_ptr<FilePicker> FilePickerFactory::createFilePicker() {
    return std::unique_ptr<FilePicker>(new ZenityFilePicker());
}
#else
std::unique_ptr<FilePicker> FilePickerFactory::createFilePicker() {
    throw std::runtime_error("No file picker implementation available");
}
#endif