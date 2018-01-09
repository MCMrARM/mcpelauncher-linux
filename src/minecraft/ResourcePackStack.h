#pragma once

#include <cstring>

class ResourcePack;
class ResourcePackRepository;

struct PackInstance {

    static void (*PackInstance_construct)(PackInstance*, ResourcePack*, int, bool);

    char filler[0x71];

    PackInstance(ResourcePack* r, int i, bool b) {
        PackInstance_construct(this, r, i, b);
    }

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