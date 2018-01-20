#pragma once

#include <memory>
#include "Resource.h"

enum class ResourceFileSystem;

class ResourceLoader {
};

class Resource {

public:
    static void registerLoader(ResourceFileSystem, std::unique_ptr<ResourceLoader>);

};