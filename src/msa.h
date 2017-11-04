#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "msa_token.h"

class MSAErrorInfo;
class MSATokenResponse;
class MSALegacyToken;
class MSALoginManager;
class MSAStorageManager;

class MSAAccount {

private:

    std::shared_ptr<MSALoginManager> manager;
    std::string username;
    std::string cid;
    std::shared_ptr<MSALegacyToken> daToken;

    std::unordered_map<MSASecurityScope, std::shared_ptr<MSAToken>> cachedTokens;

public:

    MSAAccount(std::shared_ptr<MSALoginManager> manager, std::string const& username, std::string const& cid,
               std::shared_ptr<MSALegacyToken> daToken) : manager(manager), username(username), cid(cid),
                                                          daToken(daToken) {
        //
    }

    std::unordered_map<MSASecurityScope, MSATokenResponse> requestTokens(std::vector<MSASecurityScope> const& scopes);

    std::string const& getUsername() const { return username; }
    std::string const& getCID() const { return cid; }
    std::shared_ptr<MSALegacyToken> getDaToken() const { return daToken; }
    std::unordered_map<MSASecurityScope, std::shared_ptr<MSAToken>> const& getCachedTokens() const { return cachedTokens; }

};

class MSATokenResponse {

private:
    MSASecurityScope securityScope;
    std::shared_ptr<MSAToken> token;
    std::shared_ptr<MSAErrorInfo> error;

public:
    MSATokenResponse() { }
    MSATokenResponse(MSASecurityScope scope, std::shared_ptr<MSAToken> token) : securityScope(scope), token(token) { }
    MSATokenResponse(MSASecurityScope scope, std::shared_ptr<MSAErrorInfo> error) : securityScope(scope), error(error) { }

    bool hasError() const { return token == nullptr; }
    MSASecurityScope const& getSecurityScope() const { return securityScope; }
    std::shared_ptr<MSAToken> getToken() { return token; }
    std::shared_ptr<MSAErrorInfo> getError() { return error; }

};

struct MSAErrorInfo {
    unsigned int reqStatus = 0;
    unsigned int errorStatus = 0;
    std::string flowUrl;
    std::string inlineAuthUrl;
    std::string inlineEndAuthUrl;
};

class MSADeviceAuth {

private:

    static std::string generateRandomCredential(const char* allowedChars, int length);

public:
    std::string membername;
    std::string password;
    std::string puid;
    std::shared_ptr<MSALegacyToken> token;

    MSADeviceAuth() { }
    MSADeviceAuth(std::string membername, std::string password, std::string puid, std::shared_ptr<MSALegacyToken> token)
            : membername(membername), password(password), puid(puid), token(token) { }

    static MSADeviceAuth generateRandom();

};

class MSALoginManager {

private:
    std::shared_ptr<MSAStorageManager> storageManager;
    MSADeviceAuth deviceAuth;
    bool hasReadDeviceAuth = false;

public:

    MSALoginManager(std::shared_ptr<MSAStorageManager> storageManager) : storageManager(storageManager) { }

    std::shared_ptr<MSAStorageManager> getStorageManager() const { return storageManager; }

    MSADeviceAuth const& requestDeviceAuth();

};


class MSAStorageManager {

public:

    virtual void readDeviceAuthInfo(MSALoginManager& manager, MSADeviceAuth& deviceAuth) { }
    virtual void onDeviceAuthChanged(MSALoginManager& manager, MSADeviceAuth& deviceAuth) { }
    virtual void onAccountTokenListChanged(MSALoginManager& manager, MSAAccount& account) { }

};