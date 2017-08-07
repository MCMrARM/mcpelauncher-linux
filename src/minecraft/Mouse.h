#pragma once

class Mouse {

public:
    static void (*feed)(char button, char type, short x, short y, short dx, short dy);

};