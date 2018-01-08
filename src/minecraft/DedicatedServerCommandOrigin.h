#pragma once

#include "string.h"

class Minecraft;

class CommandOrigin {
};

class DedicatedServerCommandOrigin : public CommandOrigin {

public:

    static void (*DedicatedServerCommandOrigin_construct)(DedicatedServerCommandOrigin*, mcpe::string const&, Minecraft&);

    char filler[0x1C];

    DedicatedServerCommandOrigin(mcpe::string const& s, Minecraft& m) {
        DedicatedServerCommandOrigin_construct(this, s, m);
    }

};