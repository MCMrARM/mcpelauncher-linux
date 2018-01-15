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

    std::vector<CommandOutputMessage> const& getMessages() const;

};