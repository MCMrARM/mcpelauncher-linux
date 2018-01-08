#pragma once

class IPackTelemetry {};

class MinecraftEventing : public IPackTelemetry {

public:

    static void (*MinecraftEventing_construct)(MinecraftEventing*, mcpe::string const&);

    static void (*MinecraftEventing_init)(MinecraftEventing*);

    char filler[0x100];

    MinecraftEventing(std::string const& str) {
        MinecraftEventing_construct(this, str);
    }

    void init() {
        MinecraftEventing_init(this);
    }

};