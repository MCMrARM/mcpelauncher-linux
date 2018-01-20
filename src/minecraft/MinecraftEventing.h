#pragma once

#include "string.h"

class IPackTelemetry {};

class MinecraftEventing : public IPackTelemetry {

public:

    char filler[0x100];

    MinecraftEventing(mcpe::string const& str);

    void init();

};