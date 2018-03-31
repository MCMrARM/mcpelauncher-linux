#pragma once

#include "std/string.h"

struct gl {
    static mcpe::string getOpenGLVendor();
    static mcpe::string getOpenGLRenderer();
    static mcpe::string getOpenGLVersion();
    static mcpe::string getOpenGLExtensions();
};

namespace mce {

namespace Platform {

struct OGL {

    static void InitBindings();

};

}

}