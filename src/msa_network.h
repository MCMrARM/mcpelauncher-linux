#pragma once

#include <chrono>
#include "msa.h"

namespace rapidxml { template <typename T> class xml_node; }

class MSANetwork {

private:

    static std::chrono::milliseconds serverTimeOffset;

    static std::string escapeText(std::string str);

    static std::string send(std::string const& url, std::string const& data);

    static std::string generateTimestamp();

    static std::shared_ptr<MSALegacyToken> parseLegacyToken(rapidxml::xml_node<char>* node);

public:

    static std::chrono::system_clock::time_point getServerTime() {
        return std::chrono::system_clock::now() + serverTimeOffset;
    }

    static std::string addDevice(std::string const& membername, std::string const& password);

    static std::shared_ptr<MSALegacyToken> authenticateDevice(std::string const& membername, std::string const& password);

    static std::vector<MSATokenResponse> requestTokens(std::string const& daToken,
                                                       std::shared_ptr<MSALegacyToken> deviceToken,
                                                       std::vector<MSASecurityScope> const& scopes);


};