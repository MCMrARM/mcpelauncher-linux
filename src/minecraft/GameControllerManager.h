#pragma once

enum class GameControllerStickState;
enum class GameControllerButtonState {
    RELEASED, PRESSED
};

struct GameControllerManager {

public:

    static GameControllerManager* sGamePadManager;

    void setGameControllerConnected(int, bool);

    void feedButton(int, int, GameControllerButtonState, bool);

    void feedStick(int, int, GameControllerStickState, float, float);

    void feedTrigger(int, int, float);

    void feedJoinGame(int, bool);

};