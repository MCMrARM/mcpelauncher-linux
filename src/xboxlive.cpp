#include "xboxlive.h"

std::shared_ptr<MSALoginManager> XboxLiveHelper::msaLoginManager;

void XboxLiveHelper::initMSALoginManager() {
    msaLoginManager = std::shared_ptr<MSALoginManager>(new MSALoginManager());
}