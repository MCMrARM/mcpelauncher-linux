#pragma once

#include "App.h"

class MinecraftClient : public App {

public:
    static void (*MinecraftClient_construct)(MinecraftClient*, int, char**);
    static void (*MinecraftClient_update)(MinecraftClient*);
    static void (*MinecraftClient_setRenderingSize)(MinecraftClient*, int, int);
    static void (*MinecraftClient_setUISizeAndScale)(MinecraftClient*, int, int, float);

    char filler [0x4000-4];

    MinecraftClient(int carg, char** args) {
        MinecraftClient_construct(this, carg, args);
    }

    void update() {
        MinecraftClient_update(this);
    }

    void setRenderingSize(int w, int h) {
        MinecraftClient_setRenderingSize(this, w, h);
    }

    void setUISizeAndScale(int w, int h, float px) {
        MinecraftClient_setUISizeAndScale(this, w, h, px);
    }

};
