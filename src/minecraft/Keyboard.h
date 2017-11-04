#pragma once

class Keyboard {

public:

    static void (*Keyboard_feed)(unsigned char, int);
    static void (*Keyboard_feedText)(const std::string&, bool, unsigned char);

    static int* states;

};