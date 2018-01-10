#pragma once

#include "string.h"

class Keyboard {

public:

    static void feed(unsigned char, int);
    static void feedText(mcpe::string const&, bool, unsigned char);

    static int* states;

};