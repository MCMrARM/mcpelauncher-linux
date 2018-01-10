#pragma once

#include <cstring>

class ResourcePack;
class ResourcePackRepository;

struct PackInstance {

    char filler[0x71];

    PackInstance(ResourcePack*, int, bool);

};

struct ResourcePackStack {

    static void** ResourcePackStack_vtable;

    void** vtable;
    char filler[0x10];

    ResourcePackStack() {
        vtable = ResourcePackStack_vtable + 2;
        memset(filler, 0, sizeof(filler));
    }

    /// @symbol _ZN17ResourcePackStack3addE12PackInstanceRK22ResourcePackRepositoryb
    void add(PackInstance const& i, ResourcePackRepository const& r, bool b);

};