#pragma once

#include <memory>
#include "string.h"

class CommandOrigin;
class CommandOutputSender;

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

    static void (*MinecraftCommands_setOutputSender)(MinecraftCommands*, std::unique_ptr<CommandOutputSender>);
    static MCRESULT (*MinecraftCommands_requestCommandExecution)(MinecraftCommands*, std::unique_ptr<CommandOrigin>, mcpe::string const&, int, bool);

    void setOutputSender(std::unique_ptr<CommandOutputSender> sender) {
        MinecraftCommands_setOutputSender(this, std::move(sender));
    }

    MCRESULT requestCommandExecution(std::unique_ptr<CommandOrigin> o, mcpe::string const& s, int i, bool b) {
        MinecraftCommands_requestCommandExecution(this, std::move(o), s, i, b);
    }

};