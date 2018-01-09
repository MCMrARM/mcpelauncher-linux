#pragma once

#include <vector>
#include "string.h"

class CommandOutputMessage {

public:

    int type;
    mcpe::string messageId;
    std::vector<mcpe::string> params;

};

class CommandOutput {

public:

    static std::vector<CommandOutputMessage> const& (*CommandOutput_getMessages)(CommandOutput const*);

    std::vector<CommandOutputMessage> const& getMessages() const {
        return CommandOutput_getMessages(this);
    }

};