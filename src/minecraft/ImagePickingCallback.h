#pragma once

#include "string.h"

class ImagePickingCallback {

public:
    virtual ~ImagePickingCallback();
    virtual void onImagePickingSuccess(const mcpe::string&);
    virtual void onImagePickingCanceled();

};