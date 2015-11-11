#pragma once

enum class TextureFormat {
    U8888, U888, U565, U5551, U4444, C565, C5551, C4444
};

struct ImageData {
    int w, h;
    std::string data;
    TextureFormat format;
    int mipLevel;
};