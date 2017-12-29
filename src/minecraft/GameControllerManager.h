#pragma once

struct GameControllerManager {

public:

    static GameControllerManager* sGamePadManager;

    static void (*GameControllerManager_setGameControllerConnected)(GameControllerManager*, int, bool);
    static void (*GameControllerManager_feedButton)(GameControllerManager*, int, int, int, bool);
    static void (*GameControllerManager_feedStick)(GameControllerManager*, int, int, int, float, float);
    static void (*GameControllerManager_feedTrigger)(GameControllerManager*, int, int, float);
    static void (*GameControllerManager_feedJoinGame)(GameControllerManager*, int, bool);

    void setGameControllerConnected(int i, bool b) {
        GameControllerManager_setGameControllerConnected(this, i, b);
    }

    void feedButton(int a, int b, int c, bool d) {
        GameControllerManager_feedButton(this, a, b, c, d);
    }

    void feedStick(int a, int b, int c, float d, float e) {
        GameControllerManager_feedStick(this, a, b, c, d, e);
    }

    void feedTrigger(int a, int b, float c) {
        GameControllerManager_feedTrigger(this, a, b, c);
    }

    void feedJoinGame(int a, bool b) {
        GameControllerManager_feedJoinGame(this, a, b);
    }

};