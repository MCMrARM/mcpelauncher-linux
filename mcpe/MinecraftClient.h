#pragma once

#include "App.h"

class MinecraftClient : public App {

public:
    static void (*MinecraftClient_construct)(MinecraftClient*, int, char**);
    static void (*MinecraftClient_update)(MinecraftClient*);
    static void (*MinecraftClient_setSize)(MinecraftClient*, int, int, float);

    char filler [0x1F4-4];

    MinecraftClient(int carg, char** args) {
        MinecraftClient_construct(this, carg, args);
    }

    void update() {
        MinecraftClient_update(this);
    }

    void setSize(int w, int h, float px) {
        MinecraftClient_setSize(this, w, h, px);
    }

};