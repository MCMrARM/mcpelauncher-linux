#pragma once

#include <string>
#include <chrono>

struct MSASecurityScope {
    std::string address;
    std::string policyRef;

    bool operator==(MSASecurityScope const& s) const {
        return address == s.address && policyRef == s.policyRef;
    }
};

class MSAToken {

public:
    using ExpireTime = std::chrono::system_clock::time_point;

protected:
    MSASecurityScope securityScope;
    ExpireTime expireTime;

public:
    MSAToken() { }
    MSAToken(MSASecurityScope const& scope, ExpireTime expire) : securityScope(scope), expireTime(expire) { }

    MSASecurityScope const& getSecurityScope() const { return securityScope; }

    bool isExpired() const { return false; } // TODO:

};

class MSALegacyToken : public MSAToken {

private:
    std::string xmlData;
    std::string binarySecret;

public:
    MSALegacyToken(std::string const& xmlData, std::string const& key) : xmlData(xmlData), binarySecret(key) { }

    MSALegacyToken(MSASecurityScope const& scope, ExpireTime expire, std::string const& xmlData,
                   std::string const& key) : MSAToken(scope, expire), xmlData(xmlData), binarySecret(key) { }

    std::string const& getXmlData() const { return xmlData; }
    std::string const& getBinarySecret() const { return binarySecret; }

};

class MSACompactToken : public MSAToken {

private:
    std::string binaryToken;

public:
    MSACompactToken(std::string const& binaryToken) : binaryToken(binaryToken) { }

    MSACompactToken(MSASecurityScope const& scope, ExpireTime expire, std::string const& binaryToken) :
            MSAToken(scope, expire), binaryToken(binaryToken) { }

    std::string const& getBinaryToken() const { return binaryToken; }

};


namespace std {
template<>
struct hash<MSASecurityScope> {
    std::size_t operator()(MSASecurityScope const& o) const {
        std::size_t const addressHash = std::hash<std::string>{}(o.address);
        std::size_t const policyRefHash = std::hash<std::string>{}(o.policyRef);
        return addressHash ^ (policyRefHash << 1);
    }
};
}