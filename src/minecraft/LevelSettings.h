#pragma once

#include <string.h>

class LevelSettings {

public:

    int seed; // 4
    int gametype; // 8
    int difficulty; // c
    bool forceGameType; // 10
    int generator; // 14
    bool hasAchievementsDisabled; // 18
    int dimension; // 1c
    int time; // 20
    bool edu; // 21
    float rainLevel, lightningLevel; // 28, 2c
    bool mpGame, lanBroadcast, xblBroadcast, commandsEnabled, texturepacksRequired, overrideSavedSettings; // 2d, 2e, 2f, 30, 31, 32~34
    char filler[0x300];

    LevelSettings();
    LevelSettings(LevelSettings const& org);

};