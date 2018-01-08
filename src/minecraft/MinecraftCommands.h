#pragma once

#include <memory>
#include "string.h"

class CommandOrigin;

enum class MCCATEGORY {
    //
};

struct MCRESULT {
    bool success;
    MCCATEGORY category;
    unsigned short code;
};

class MinecraftCommands {

public:

    static MCRESULT (*MinecraftCommands_requestCommandExecution)(MinecraftCommands*, std::unique_ptr<CommandOrigin>, mcpe::string const&, int, bool);

    MCRESULT requestCommandExecution(std::unique_ptr<CommandOrigin> o, mcpe::string const& s, int i, bool b) {
        MinecraftCommands_requestCommandExecution(this, std::move(o), s, i, b);
    }

};