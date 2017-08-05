#include "msa.h"

#include <random>
#include <cstring>
#include "msa_token.h"
#include "msa_network.h"

MSADeviceAuth const& MSALoginManager::requestDeviceAuth() {
    if (deviceAuth.membername.empty()) {
        deviceAuth = MSADeviceAuth::generateRandom();
        if (storageManager)
            storageManager->onDeviceAuthChanged(*this, deviceAuth);
    }
    if (deviceAuth.puid.empty()) {
        deviceAuth.puid = MSANetwork::addDevice(deviceAuth.membername, deviceAuth.password);
        if (storageManager)
            storageManager->onDeviceAuthChanged(*this, deviceAuth);
    }
    if (!deviceAuth.token) {
        deviceAuth.token = MSANetwork::authenticateDevice(deviceAuth.membername, deviceAuth.password);
        if (!deviceAuth.token)
            throw std::runtime_error("Failed to authenticate device");
        if (storageManager)
            storageManager->onDeviceAuthChanged(*this, deviceAuth);
    }
    return deviceAuth;
}

std::unordered_map<MSASecurityScope, MSATokenResponse> MSAAccount::requestTokens(std::vector<MSASecurityScope> const& scopes) {
    std::vector<MSASecurityScope> requestScopes;
    std::unordered_map<MSASecurityScope, MSATokenResponse> ret;
    for (MSASecurityScope const& scope : scopes) {
        if (cachedTokens.count(scope) > 0 && !cachedTokens[{scope.address}]->isExpired()) {
            ret[scope] = MSATokenResponse(scope, cachedTokens[{scope.address}]);
            continue;
        }
        requestScopes.push_back(scope);
    }
    std::vector<MSATokenResponse> resp = MSANetwork::requestTokens(daToken, manager->requestDeviceAuth().token, scopes);
    bool hasNewTokens = false;
    for (MSATokenResponse& token : resp) {
        if (!token.hasError()) {
            cachedTokens[token.getSecurityScope()] = token.getToken();
            hasNewTokens = true;
        }
        ret[token.getSecurityScope()] = token;
    }
    if (hasNewTokens && manager->storageManager)
        manager->storageManager->onAccountTokenListChanged(*manager, *this);
    return ret;
}

MSADeviceAuth MSADeviceAuth::generateRandom() {
    std::string membername = generateRandomCredential("abcdefghijklmnopqrstuvwxyz", 18);
    std::string password = generateRandomCredential("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}/?;:'\\\",.<>`~", 16);
    return MSADeviceAuth(membername, password, std::string(), std::shared_ptr<MSALegacyToken>());
}

std::string MSADeviceAuth::generateRandomCredential(const char* allowedChars, int length) {
    std::string ret;
    ret.resize((size_t) length);
    std::uniform_int_distribution<int> d(0, strlen(allowedChars) - 1);
    std::random_device rd;
    for (int i = 0; i < length; i++)
        ret[i] = allowedChars[d(rd)];
    return ret;
}