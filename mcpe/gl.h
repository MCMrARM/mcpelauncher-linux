#pragma once

struct gl {
    static std::string (*getOpenGLVendor)();
    static std::string (*getOpenGLRenderer)();
    static std::string (*getOpenGLVersion)();
    static std::string (*getOpenGLExtensions)();
};

namespace mce {

namespace Platform {

struct OGL {

    static void (*OGL_initBindings)();
    static void initBindings() { OGL_initBindings(); }

};

}

}