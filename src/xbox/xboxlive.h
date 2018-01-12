#pragma once

#include "msa.h"
#include "../common/cll.h"
#include "../minecraft/Xbox.h"
#include <map>

class SimpleMSAStorageManager;

class XboxLiveHelper {

private:

    static std::shared_ptr<CLL> cll;
    static std::shared_ptr<MSALoginManager> msaLoginManager;
    static std::shared_ptr<SimpleMSAStorageManager> msaStorageManager;

    static void initMSALoginManager();

public:

    static std::shared_ptr<CLL> getCLL() {
        if (!msaLoginManager)
            initMSALoginManager();
        return cll;
    }

    static std::shared_ptr<MSALoginManager> getMSALoginManager() {
        if (!msaLoginManager)
            initMSALoginManager();
        return msaLoginManager;
    }

    static std::shared_ptr<SimpleMSAStorageManager> getMSAStorageManager() {
        if (!msaLoginManager)
            initMSALoginManager();
        return msaStorageManager;
    }

    static void shutdown();

    static void invokeXbLogin(xbox::services::system::user_auth_android* auth, std::string const& binaryToken);

};

class SimpleMSAStorageManager : public MSAStorageManager {

private:

    static const std::string DEVICE_AUTH_PATH;
    static const std::string ACCOUNT_INFO_PATH;

    static std::map<std::string, std::string> readProperties(std::istream& stream);
    static void writeProperties(std::ostream& stream, std::map<std::string, std::string> const& properties);

    std::shared_ptr<MSAAccount> account;

public:
    void setAccount(std::shared_ptr<MSAAccount> account);

    std::shared_ptr<MSAAccount> getAccount();

    virtual void readDeviceAuthInfo(MSALoginManager& manager, MSADeviceAuth& deviceAuth);
    virtual void onDeviceAuthChanged(MSALoginManager& manager, MSADeviceAuth& deviceAuth);
    virtual void onAccountTokenListChanged(MSALoginManager& manager, MSAAccount& account);
    void onAccountInfoChanged();

};