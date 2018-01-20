#pragma once

#include <memory>

namespace Social {

struct MultiplayerService {
    char filler[0x128];
};

struct MultiplayerXBL : public MultiplayerService, public std::enable_shared_from_this<MultiplayerXBL> {

    char filler[0x200];

    MultiplayerXBL();

};

}