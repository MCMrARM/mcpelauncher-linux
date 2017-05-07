#pragma once

#include "App.h"

class Options;

class MinecraftGame : public App {

public:
    static void (*MinecraftGame_construct)(MinecraftGame*, int, char**);
    static void (*MinecraftGame_update)(MinecraftGame*);
    static void (*MinecraftGame_setRenderingSize)(MinecraftGame*, int, int);
    static void (*MinecraftGame_setUISizeAndScale)(MinecraftGame*, int, int, float);
    static Options* (*MinecraftGame_getOptions)(MinecraftGame*);

    char filler [0x4000-4];

    MinecraftGame(int carg, char** args) {
        MinecraftGame_construct(this, carg, args);
    }

    void update() {
        MinecraftGame_update(this);
    }

    void setRenderingSize(int w, int h) {
        MinecraftGame_setRenderingSize(this, w, h);
    }

    void setUISizeAndScale(int w, int h, float px) {
        MinecraftGame_setUISizeAndScale(this, w, h, px);
    }

    Options* getOptions() {
        return MinecraftGame_getOptions(this);
    }

};
