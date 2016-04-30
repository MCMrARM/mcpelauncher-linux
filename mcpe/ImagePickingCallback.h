#pragma once

#include <string>

class ImagePickingCallback {

public:
    virtual ~ImagePickingCallback();
    virtual void onImagePickingSuccess(const std::string&);
    virtual void onImagePickingCanceled();

};