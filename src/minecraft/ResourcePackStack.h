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

    static void (*ResourcePackStack_add)(ResourcePackStack*, PackInstance const&, ResourcePackRepository const&, bool);

    void** vtable;
    char filler[0x10];

    ResourcePackStack() {
        vtable = ResourcePackStack_vtable + 2;
        memset(filler, 0, sizeof(filler));
    }

    void add(PackInstance const& i, ResourcePackRepository const& r, bool b) {
        ResourcePackStack_add(this, i, r, b);
    }

};