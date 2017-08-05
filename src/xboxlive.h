#pragma once

#include "msa.h"

class XboxLiveHelper {

private:

    static std::shared_ptr<MSALoginManager> msaLoginManager;

    static void initMSALoginManager();

public:

    static std::shared_ptr<MSALoginManager> getMSALoginManager() {
        if (!msaLoginManager)
            initMSALoginManager();
        return msaLoginManager;
    }

};