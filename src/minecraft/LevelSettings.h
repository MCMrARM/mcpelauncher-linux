#pragma once

#include <string.h>

class LevelSettings {

public:

    static void (*LevelSettings_construct)(LevelSettings*);
    static void (*LevelSettings_construct2)(LevelSettings*, LevelSettings const&);

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

    LevelSettings() {
        LevelSettings_construct(this);
    }
    LevelSettings(LevelSettings const& org) {
        //memcpy((void*) this, (void const*) &org, sizeof(LevelSettings));
        LevelSettings_construct2(this, org);
    }

};