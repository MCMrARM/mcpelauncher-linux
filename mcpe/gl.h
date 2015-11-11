#pragma once

struct gl {
    static std::string (*getOpenGLVendor)();
    static std::string (*getOpenGLRenderer)();
    static std::string (*getOpenGLVersion)();
    static std::string (*getOpenGLExtensions)();
};