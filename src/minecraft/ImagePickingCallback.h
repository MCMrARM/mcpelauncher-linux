#pragma once

#include "std/string.h"

class ImagePickingCallback {

public:
    virtual ~ImagePickingCallback() = 0;
    virtual void onImagePickingSuccess(const mcpe::string&) = 0;
    virtual void onImagePickingCanceled() = 0;

};