#pragma once

#include <memory>
#include "App.h"

class Options;

class MinecraftGame : public App {

public:
    static void (*MinecraftGame_construct)(MinecraftGame*, int, char**);
    static void (*MinecraftGame_destruct)(MinecraftGame*);
    static void (*MinecraftGame_update)(MinecraftGame*);
    static void (*MinecraftGame_setRenderingSize)(MinecraftGame*, int, int);
    static void (*MinecraftGame_setUISizeAndScale)(MinecraftGame*, int, int, float);
    static std::shared_ptr<Options> (*MinecraftGame_getPrimaryUserOptions)(MinecraftGame*);

    char filler [0x4000-4];

    MinecraftGame(int carg, char** args) {
        MinecraftGame_construct(this, carg, args);
    }

    ~MinecraftGame() {
        MinecraftGame_destruct(this);
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

    std::shared_ptr<Options> getPrimaryUserOptions() {
        return MinecraftGame_getPrimaryUserOptions(this);
    }

};
