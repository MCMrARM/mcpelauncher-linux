#pragma once

struct KeyboardAction {
    int action;
    int keyCode;
};

class Keyboard {

public:

    static void (*Keyboard_feedText)(const std::string&, bool, unsigned char);

    static std::vector<KeyboardAction>* inputs;
    static int* states;

};