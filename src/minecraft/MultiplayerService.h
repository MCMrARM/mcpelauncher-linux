#pragma once

#include <memory>

namespace Social {

struct MultiplayerService {
    char filler[0x130];
};

struct MultiplayerXBL : public MultiplayerService, public std::enable_shared_from_this<MultiplayerXBL> {

    char filler[0x200];

    MultiplayerXBL();

};

}