#pragma once

#include <memory>
#include "App.h"

class Options;

class MinecraftGame : public App {

public:

    char filler [0x4000];

    MinecraftGame(int carg, char** args);

    ~MinecraftGame();

    void update();

    void setRenderingSize(int, int);

    void setUISizeAndScale(int, int, float);

    std::shared_ptr<Options> getPrimaryUserOptions();

};
