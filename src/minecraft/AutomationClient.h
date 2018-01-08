#pragma once

namespace Automation {

class AutomationClient {

public:

    static void (*AutomationClient_construct)(AutomationClient*, IMinecraftApp&);

    char filler[0x300];

    AutomationClient(IMinecraftApp& a) {
        AutomationClient_construct(this, a);
    }


};

}