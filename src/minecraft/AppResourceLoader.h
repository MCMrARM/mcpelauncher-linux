#pragma once

#include "string.h"
#include <functional>

#include "Resource.h"

class AppResourceLoader : public ResourceLoader {

private:
    char filler[0x14];

public:
    /// @symbol _ZN17AppResourceLoaderC2ESt8functionIFSsvEE
    AppResourceLoader(std::function<mcpe::string ()>);

};