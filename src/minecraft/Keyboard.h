#pragma once

#include "string.h"

class Keyboard {

public:

    static void (*Keyboard_feed)(unsigned char, int);
    static void (*Keyboard_feedText)(const mcpe::string&, bool, unsigned char);

    static int* states;

};