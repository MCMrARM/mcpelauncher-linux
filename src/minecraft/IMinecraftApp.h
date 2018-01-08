#pragma once

class Minecraft;

namespace Automation {
    class AutomationClient;
}

class IMinecraftApp {

public:

    virtual ~IMinecraftApp() { }
    virtual Minecraft* getPrimaryMinecraft() = 0;
    virtual Automation::AutomationClient* getAutomationClient() = 0;
    virtual bool isEduMode() = 0;
    virtual bool isDedicatedServer() = 0;
    virtual int getDefaultNetworkMaxPlayers() = 0;
    virtual void onNetworkMaxPlayersChanged(unsigned int) = 0;

};