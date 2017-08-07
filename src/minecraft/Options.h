#pragma once

class Options {

public:
    static bool (*Options_getFullscreen)(Options*);
    static void (*Options_setFullscreen)(Options*, bool);

    bool getFullscreen() {
        return Options_getFullscreen(this);
    }
    void setFullscreen(bool b) {
        Options_setFullscreen(this, b);
    }

};