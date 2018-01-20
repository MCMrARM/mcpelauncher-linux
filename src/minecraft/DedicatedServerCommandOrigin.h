#pragma once

#include "string.h"

class Minecraft;

class CommandOrigin {
};

class DedicatedServerCommandOrigin : public CommandOrigin {

public:

    char filler[0x1C];

    DedicatedServerCommandOrigin(mcpe::string const& s, Minecraft& m);

};