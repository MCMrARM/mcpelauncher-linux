#pragma once

#include "../minecraft/IMinecraftApp.h"

class DedicatedServerMinecraftApp : public IMinecraftApp {

public:
    Automation::AutomationClient* automationClient;

    virtual Minecraft* getPrimaryMinecraft() { return nullptr; }
    virtual Automation::AutomationClient* getAutomationClient() { return automationClient; }
    virtual bool isEduMode() { return false; }
    virtual bool isDedicatedServer() { return true; }
    virtual int getDefaultNetworkMaxPlayers() { return 20; }
    virtual void onNetworkMaxPlayersChanged(unsigned int max) {}

};