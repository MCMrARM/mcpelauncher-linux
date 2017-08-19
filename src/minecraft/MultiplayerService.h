#pragma once

namespace Social {

struct MultiplayerService {
    //
};

struct MultiplayerXBL : public MultiplayerService {

    static void (*MultiplayerXBL_MultiplayerXBL)(MultiplayerXBL*);

    char filler[0x200];

    MultiplayerXBL() {
        MultiplayerXBL_MultiplayerXBL(this);
    }

};

}