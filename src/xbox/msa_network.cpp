#include "msa_network.h"

#include <sstream>
#include <cstring>
#include <iostream>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <random>
#include <netinet/in.h>
#include "../common/base64.h"
#include "../common/log.h"

std::chrono::milliseconds MSANetwork::serverTimeOffset;

static size_t curl_stringstream_write_func(void* ptr, size_t size, size_t nmemb, std::stringstream* s) {
    s->write((char*) ptr, size * nmemb);
    return size * nmemb;
}
static size_t curl_header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    // "Date: "
    if (size * nitems > 6 && memcmp(buffer, "Date: ", 6) == 0) {
        std::string s (&buffer[6], nitems * size - 6);
        struct tm tm;
        strptime(s.c_str(), "%a, %d %b %Y %H:%M:%S", &tm);
        auto serverTime = std::chrono::system_clock::from_time_t(timegm(&tm));
        auto localTime = std::chrono::system_clock::now();
        MSANetwork::serverTimeOffset = std::chrono::duration_cast<std::chrono::milliseconds>(serverTime.time_since_epoch()) - std::chrono::duration_cast<std::chrono::milliseconds>(localTime.time_since_epoch());
    }
    return nitems * size;
}

std::string MSANetwork::send(std::string const& url, std::string const& data) {
    Log::trace("MSANetwork", "Send %s: %s", url.c_str(), data.c_str());

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) data.length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, curl_header_callback);

    std::stringstream output;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_stringstream_write_func);
    curl_easy_perform(curl);
    Log::trace("MSANetwork", "Reply: %s", output.str().c_str());
    return output.str();
}

std::string MSANetwork::escapeText(std::string str) {
    for (ssize_t i = (ssize_t) str.length() - 1; i >= 0; --i) {
        if (str[i] == '<')
            str.replace((size_t) i, 1, "&lt;");
        else if (str[i] == '&')
            str.replace((size_t) i, 1, "&amp;");
    }
    return std::move(str);
}

std::string MSANetwork::escapeURL(std::string str) {
    CURL* curl = curl_easy_init();
    char* escaped = curl_easy_escape(curl, str.data(), str.length());
    std::string ret (escaped);
    curl_free(escaped);
    return ret;
}

std::string MSANetwork::generateTimestamp() {
    std::chrono::system_clock::time_point created = getServerTime();

    char createdTimestamp[24];
    time_t time = std::chrono::system_clock::to_time_t(created);
    strftime(createdTimestamp, sizeof(createdTimestamp), "%FT%TZ", gmtime(&time));
    char expiresTimestamp[24];
    time = std::chrono::system_clock::to_time_t(created + std::chrono::minutes(5));
    strftime(expiresTimestamp, sizeof(expiresTimestamp), "%FT%TZ", gmtime(&time));

    std::stringstream ss;
    ss << "<wsu:Timestamp xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" wsu:Id=\"Timestamp\">"
       <<   "<wsu:Created>" << createdTimestamp << "</wsu:Created>"
       <<   "<wsu:Expires>" << expiresTimestamp << "</wsu:Expires>"
       << "</wsu:Timestamp>";
    return ss.str();
}

std::string MSANetwork::generateKey(int keyLength, std::string const& sessionKey, std::string const& keyUsage,
                                    std::string const& nonce) {
    std::string ret;
    ret.reserve((size_t) keyLength);
    unsigned char* buf = new unsigned char[sizeof(int) + keyUsage.length() + sizeof(char) + nonce.length() + sizeof(int)];
    unsigned char resultBuf[EVP_MAX_MD_SIZE];
    size_t resultSize = 0;
    size_t off = 0;
    off += sizeof(int);
    memcpy(&buf[off], keyUsage.data(), keyUsage.length()); off += keyUsage.length();
    buf[off] = 0; off++;
    memcpy(&buf[off], nonce.data(), nonce.length()); off += nonce.length();
    ((int&) buf[off]) = htonl((uint32_t) (keyLength * 8)); off += sizeof(int);
    size_t i = 1;
    while (ret.length() < keyLength) {
        ((int&) buf[0]) = htonl(i++);
        #ifdef __APPLE__
        HMAC(EVP_sha256(), sessionKey.data(), sessionKey.length(), buf, off, resultBuf, (unsigned int*)&resultSize);
        #elif
        HMAC(EVP_sha256(), sessionKey.data(), sessionKey.length(), buf, off, resultBuf, &resultSize);
        #endif
        ret.append((char*) resultBuf, std::min<size_t>(resultSize, keyLength - ret.size()));
    }
    delete[] buf;
    return ret;
}

std::string MSANetwork::generateNonce() {
    std::string ret;
    ret.resize(32);
    std::random_device rd;
    unsigned int* data = (unsigned int*) &ret[0];
    for (int i = 0; i < 32 / sizeof(unsigned int); i++)
        data[i] = rd();
    return ret;
}

std::string MSANetwork::generateDeviceDAToken(std::shared_ptr<MSALegacyToken> deviceToken) {
    using namespace std::chrono;
    std::stringstream ss;
    std::string nonce = generateNonce();
    ss << "ct=" << std::to_string(duration_cast<seconds>(getServerTime().time_since_epoch()).count())
       << "&hashalg=SHA256" << "&bver=11" << "&appid=%7BF501FD64-9070-46AB-993C-6F7B71D8D883%7D&"
       << "da=" << escapeURL(deviceToken->getXmlData()) + "&nonce=" + escapeURL(Base64::encode(nonce));
    std::string data = ss.str();
    ss << "&hash=" << escapeURL(createSignature(data, deviceToken->getBinarySecret(), "WS-SecureConversation", nonce));
    return ss.str();
}

std::string MSANetwork::generateSignedInfoBlock(std::string const& section, std::string const& text) {
    std::string hash;
    hash.resize(SHA256_DIGEST_LENGTH);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, text.data(), text.length());
    SHA256_Final((unsigned char*) &hash[0], &ctx);

    std::stringstream ss;
    ss << "<Reference URI=\"#" << section << "\">"
       <<   "<Transforms>"
       <<     "<Transform Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\"></Transform>"
       <<   "</Transforms>"
       <<   "<DigestMethod Algorithm=\"http://www.w3.org/2001/04/xmlenc#sha256\"></DigestMethod>"
       <<   "<DigestValue>" << Base64::encode(hash) << "</DigestValue>"
       << "</Reference>";
    return ss.str();
}

std::string MSANetwork::createSignature(std::string const& data, std::string const& binarySecret,
                                        std::string const& keyUsage, std::string const& nonce) {
    std::string signatureKey = generateKey(32, binarySecret, keyUsage, nonce);
    unsigned char signatureBuf[EVP_MAX_MD_SIZE];
    size_t signatureSize = 0;
    #ifdef __APPLE__
    HMAC(EVP_sha256(), signatureKey.data(), signatureKey.length(), (unsigned char*) data.data(), data.size(), signatureBuf, (unsigned int*)&signatureSize);
    #elif
    HMAC(EVP_sha256(), signatureKey.data(), signatureKey.length(), (unsigned char*) data.data(), data.size(), signatureBuf, &signatureSize);
    #endif

    return Base64::encode(std::string((char*) signatureBuf, signatureSize));
}

std::string MSANetwork::decryptData(rapidxml::xml_node<char>* node, std::string const& binarySecret,
                                    std::string const& nonce) {
    rapidxml::xml_node<char>* cipherData = node->first_node("CipherData");
    if (cipherData == nullptr)
        throw std::runtime_error("Failed to find CipherData");
    rapidxml::xml_node<char>* cipherValue = cipherData->first_node("CipherValue");
    if (cipherValue == nullptr)
        throw std::runtime_error("Failed to find CipherValue");
    std::string data = Base64::decode(std::string(cipherValue->value(), cipherValue->value_size()));
    std::string decryptedData;
    decryptedData.resize(data.size());

    std::string keyData = generateKey(32, binarySecret, "WS-SecureConversationWS-SecureConversation", nonce);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!EVP_DecryptInit(ctx, EVP_aes_256_cbc(), (unsigned char*) keyData.data(), (unsigned char*) &data[0]))
        throw std::runtime_error("EVP_DecryptInit failed");
    size_t len = 0;
    int tempLen;
    if (!EVP_DecryptUpdate(ctx, (unsigned char*) &decryptedData[0], &tempLen, (unsigned char*) &data[16], data.size() - 16))
        throw std::runtime_error("EVP_DecryptUpdate failed");
    len += tempLen;
    if (!EVP_DecryptFinal_ex(ctx, (unsigned char*) &decryptedData[len], &tempLen)) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error("EVP_DecryptFinal failed");
    }
    len += tempLen;
    EVP_CIPHER_CTX_free(ctx);
    decryptedData.resize(len);
    return decryptedData;
}

MSATokenResponse MSANetwork::parseTokenResponse(rapidxml::xml_node<char>* node) {
    rapidxml::xml_node<char>* appliesTo = node->first_node("wsp:AppliesTo");
    if (appliesTo == nullptr)
        throw std::runtime_error("Failed to find wsp:AppliesTo");
    rapidxml::xml_node<char>* appliesToEndpointRef = appliesTo->first_node("wsa:EndpointReference");
    if (appliesToEndpointRef == nullptr)
        throw std::runtime_error("Failed to find wsa:EndpointReference");
    rapidxml::xml_node<char>* address = appliesToEndpointRef->first_node("wsa:Address");
    if (address == nullptr)
        throw std::runtime_error("Failed to find wsa:Address");

    MSASecurityScope scope = {std::string(address->value(), address->value_size())};

    rapidxml::xml_node<char>* tokenType = node->first_node("wst:TokenType");
    if (tokenType == nullptr) {
        rapidxml::xml_node<char>* psf = node->first_node("psf:pp");
        return MSATokenResponse(scope, parseErrorInfo(psf));
    }

    MSAToken::ExpireTime expire;
    rapidxml::xml_node<char>* lifetime = node->first_node("wst:Lifetime");
    if (lifetime != nullptr) {
        rapidxml::xml_node<char>* expires = node->first_node("wsu:Expires");
        struct tm tm;
        if (expires != nullptr && strptime(expires->value(), "%FT%TZ", &tm))
            expire = std::chrono::system_clock::from_time_t(timegm(&tm));
    }

    if (strcmp(tokenType->value(), "urn:passport:legacy") == 0)
        return MSATokenResponse(scope, parseLegacyToken(node, scope, expire));
    else if (strcmp(tokenType->value(), "urn:passport:compact") == 0)
        return MSATokenResponse(scope, parseCompactToken(node, scope, expire));

    return MSATokenResponse(scope, std::shared_ptr<MSAErrorInfo>(new MSAErrorInfo()));
}

std::shared_ptr<MSALegacyToken> MSANetwork::parseLegacyToken(rapidxml::xml_node<char>* node, MSASecurityScope scope,
                                                             MSAToken::ExpireTime expire) {
    rapidxml::xml_node<char>* requested = node->first_node("wst:RequestedSecurityToken");
    if (requested == nullptr)
        throw std::runtime_error("Failed to find wst:RequestedSecurityToken");
    rapidxml::xml_node<char>* data = requested->first_node("EncryptedData");
    if (data == nullptr)
        throw std::runtime_error("Failed to find EncryptedData");
    rapidxml::xml_node<char>* proof = node->first_node("wst:RequestedProofToken");
    if (proof == nullptr)
        throw std::runtime_error("Failed to find wst:RequestedProofToken");
    rapidxml::xml_node<char>* binarySecret = proof->first_node("wst:BinarySecret");
    if (binarySecret == nullptr)
        throw std::runtime_error("Failed to find wst:BinarySecret");

    std::stringstream ss;
    rapidxml::print_to_stream(ss, *data, rapidxml::print_no_indenting);
    return std::shared_ptr<MSALegacyToken>(new MSALegacyToken(scope, expire, ss.str(), Base64::decode(binarySecret->value())));
}

std::shared_ptr<MSACompactToken> MSANetwork::parseCompactToken(rapidxml::xml_node<char>* node, MSASecurityScope scope,
                                                               MSAToken::ExpireTime expire) {
    rapidxml::xml_node<char>* requested = node->first_node("wst:RequestedSecurityToken");
    if (requested == nullptr)
        throw std::runtime_error("Failed to find wst:RequestedSecurityToken");
    rapidxml::xml_node<char>* data = requested->first_node("wsse:BinarySecurityToken");
    if (data == nullptr)
        throw std::runtime_error("Failed to find wsse:BinarySecurityToken");
    return std::shared_ptr<MSACompactToken>(new MSACompactToken(scope, expire, data->value()));
}


std::shared_ptr<MSAErrorInfo> MSANetwork::parseErrorInfo(rapidxml::xml_node<char>* node) {
    std::shared_ptr<MSAErrorInfo> ret (new MSAErrorInfo());
    if (node == nullptr)
        return ret;
    rapidxml::xml_node<char>* reqStatus = node->first_node("psf:reqstatus");
    if (reqStatus != nullptr)
        ret->reqStatus = (unsigned int) atoi(reqStatus->value());
    rapidxml::xml_node<char>* errorStatus = node->first_node("psf:errorstatus");
    if (errorStatus != nullptr)
        ret->errorStatus = (unsigned int) atoi(errorStatus->value());
    rapidxml::xml_node<char>* flowUrl = node->first_node("psf:flowurl");
    if (flowUrl != nullptr)
        ret->flowUrl = flowUrl->value();
    rapidxml::xml_node<char>* inlineAuthUrl = node->first_node("psf:inlineauthurl");
    if (inlineAuthUrl != nullptr)
        ret->inlineAuthUrl = inlineAuthUrl->value();
    rapidxml::xml_node<char>* inlineEndAuthUrl = node->first_node("psf:inlineendauthurl");
    if (inlineEndAuthUrl != nullptr)
        ret->inlineEndAuthUrl = inlineEndAuthUrl->value();
    return ret;
}

std::string MSANetwork::addDevice(std::string const& membername, std::string const& password) {
    std::stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
       << "<DeviceAddRequest>"
       <<   "<ClientInfo name=\"MSAAndroidApp\" version=\"1.0\"/>"
       <<     "<Authentication>"
       <<     "<Membername>" << escapeText(membername) << "</Membername>"
       <<     "<Password>" + escapeText(password) + "</Password>"
       <<   "</Authentication>"
       << "</DeviceAddRequest>";
    std::string rep = send("https://login.live.com/ppsecure/deviceaddcredential.srf", ss.str());
    ss.clear();

    rapidxml::xml_document<char> doc;
    doc.parse<0>(&rep[0]);
    rapidxml::xml_node<char>* root = doc.first_node("DeviceAddResponse");
    if (root == nullptr)
        throw std::runtime_error("Failed to get the root node");
    rapidxml::xml_node<char>* success = root->first_node("success");
    if (success == nullptr || strcmp(success->value(), "true") != 0)
        throw std::runtime_error("Failed to add device (success != true)");
    rapidxml::xml_node<char>* puid = root->first_node("puid");
    if (puid == nullptr)
        throw std::runtime_error("Failed to find the puid field");
    return puid->value();
}

std::shared_ptr<MSALegacyToken> MSANetwork::authenticateDevice(std::string const& membername,
                                                               std::string const& password) {
    using namespace std::chrono;
    milliseconds messageID = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    std::stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
       << "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\" xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2005/02/sc\" xmlns:wst=\"http://schemas.xmlsoap.org/ws/2005/02/trust\">"
       <<   "<s:Header>"
       <<     "<wsa:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/02/trust/RST/Issue</wsa:Action>"
       <<     "<wsa:To s:mustUnderstand=\"1\">https://login.live.com/RST2.srf</wsa:To>"
       <<     "<wsa:MessageID>" << std::to_string(messageID.count()) << "</wsa:MessageID>"
       <<     "<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"
       <<       "<ps:BinaryVersion>11</ps:BinaryVersion>"
       <<       "<ps:DeviceType>Android</ps:DeviceType>"
       <<       "<ps:HostingApp>{F501FD64-9070-46AB-993C-6F7B71D8D883}</ps:HostingApp>"
       <<     "</ps:AuthInfo>"
       <<     "<wsse:Security>"
       <<       "<wsse:UsernameToken wsu:Id=\"devicesoftware\">"
       <<         "<wsse:Username>" << escapeText(membername) << "</wsse:Username>"
       <<         "<wsse:Password>" << escapeText(password) << "</wsse:Password>"
       <<       "</wsse:UsernameToken>"
       <<       generateTimestamp()
       <<     "</wsse:Security>"
       <<   "</s:Header>"
       <<   "<s:Body>"
       <<   "<wst:RequestSecurityToken xmlns:wst=\"http://schemas.xmlsoap.org/ws/2005/02/trust\" Id=\"RST0\">"
       <<     "<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
       <<     "<wsp:AppliesTo xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\">"
       <<       "<wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">"
       <<       "<wsa:Address>http://Passport.NET/tb</wsa:Address></wsa:EndpointReference>"
       <<     "</wsp:AppliesTo>"
       <<   "</wst:RequestSecurityToken>"
       <<   "</s:Body>"
       << "</s:Envelope>";


    std::string rep = send("https://login.live.com/RST2.srf", ss.str());
    ss.clear();

    rapidxml::xml_document<char> doc;
    doc.parse<0>(&rep[0]);
    rapidxml::xml_node<char>* root = doc.first_node("S:Envelope");
    if (root == nullptr)
        throw std::runtime_error("Failed to get the root node");
    rapidxml::xml_node<char>* body = root->first_node("S:Body");
    if (body == nullptr)
        throw std::runtime_error("Failed to get the body node");
    rapidxml::xml_node<char>* token = body->first_node("wst:RequestSecurityTokenResponse");
    if (token == nullptr)
        throw std::runtime_error("Failed to find the token");
    return parseLegacyToken(token, MSASecurityScope(), MSAToken::ExpireTime());
}

std::vector<MSATokenResponse> MSANetwork::requestTokens(std::shared_ptr<MSALegacyToken> daToken,
                                                        std::shared_ptr<MSALegacyToken> deviceToken,
                                                        std::vector<MSASecurityScope> const& scopes) {
    using namespace std::chrono;
    milliseconds messageID = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

    std::string nonce = generateNonce();
    std::string timestamp = generateTimestamp();

    std::stringstream rs;
    rs << "<ps:RequestMultipleSecurityTokens xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"RSTS\">";
    int tokenId = 0;
    for (MSASecurityScope const& scope : scopes) {
        rs << "<wst:RequestSecurityToken xmlns:wst=\"http://schemas.xmlsoap.org/ws/2005/02/trust\" Id=\"RST" << tokenId << "\">"
           <<   "<wst:RequestType>http://schemas.xmlsoap.org/ws/2005/02/trust/Issue</wst:RequestType>"
           <<   "<wsp:AppliesTo xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\">"
           <<     "<wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">"
           <<       "<wsa:Address>" << scope.address << "</wsa:Address>"
           <<     "</wsa:EndpointReference>"
           <<   "</wsp:AppliesTo>";
        if (!scope.policyRef.empty())
            rs << "<wsp:PolicyReference xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\" URI=\"" << scope.policyRef << "\"></wsp:PolicyReference>";
        rs << "</wst:RequestSecurityToken>";
    }
    rs << "</ps:RequestMultipleSecurityTokens>";

    std::stringstream ss;
    std::stringstream as;
    std::stringstream hs;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
       << "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" xmlns:wsse=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-secext-1.0.xsd\" xmlns:saml=\"urn:oasis:names:tc:SAML:1.0:assertion\" xmlns:wsp=\"http://schemas.xmlsoap.org/ws/2004/09/policy\" xmlns:wsu=\"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd\" xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" xmlns:wssc=\"http://schemas.xmlsoap.org/ws/2005/02/sc\" xmlns:wst=\"http://schemas.xmlsoap.org/ws/2005/02/trust\">"
       <<   "<s:Header>"
       <<     "<wsa:Action s:mustUnderstand=\"1\">http://schemas.xmlsoap.org/ws/2005/02/trust/RST/Issue</wsa:Action>"
       <<     "<wsa:To s:mustUnderstand=\"1\">https://login.live.com/RST2.srf</wsa:To>"
       <<     "<wsa:MessageID>" << std::to_string(messageID.count()) << "</wsa:MessageID>";
    as <<     "<ps:AuthInfo xmlns:ps=\"http://schemas.microsoft.com/Passport/SoapServices/PPCRL\" Id=\"PPAuthInfo\">"
       <<       "<ps:BinaryVersion>11</ps:BinaryVersion>"
       <<       "<ps:DeviceType>Android</ps:DeviceType>"
       <<       "<ps:HostingApp>{F501FD64-9070-46AB-993C-6F7B71D8D883}</ps:HostingApp>"
       <<       "<ps:InlineUX>Android</ps:InlineUX>"
       <<       "<ps:ConsentFlags>1</ps:ConsentFlags>"
       <<       "<ps:IsConnected>1</ps:IsConnected>"
       <<       "<ps:ClientAppURI>android-app://com.mojang.minecraftpe.H62DKCBHJP6WXXIV7RBFOGOL4NAK4E6Y</ps:ClientAppURI>"
       <<     "</ps:AuthInfo>";
    ss <<     as.str()
       <<     "<wsse:Security>"
       <<       daToken->getXmlData()
       <<       "<wsse:BinarySecurityToken ValueType=\"urn:liveid:sha1device\" Id=\"DeviceDAToken\">" << escapeText(generateDeviceDAToken(deviceToken)) << "</wsse:BinarySecurityToken>"
       <<       "<wssc:DerivedKeyToken wsu:Id=\"SignKey\" Algorithm=\"urn:liveid:SP800-108CTR-HMAC-SHA256\">"
       <<         "<wsse:RequestedTokenReference>"
       <<           "<wsse:KeyIdentifier ValueType=\"http://docs.oasis-open.org/wss/2004/XX/oasis-2004XX-wss-saml-token-profile-1.0#SAMLAssertionID\"/>"
       <<           "<wsse:Reference URI=\"\"/>"
       <<         "</wsse:RequestedTokenReference>"
       <<         "<wssc:Nonce>" << Base64::encode(nonce) << "</wssc:Nonce>"
       <<       "</wssc:DerivedKeyToken>"
       <<       timestamp;
    hs << "<SignedInfo xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"
       <<   "<CanonicalizationMethod Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\"></CanonicalizationMethod>"
       <<   "<SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#hmac-sha256\"></SignatureMethod>"
       <<   generateSignedInfoBlock("PPAuthInfo", as.str())
       <<   generateSignedInfoBlock("Timestamp", timestamp)
       <<   generateSignedInfoBlock("RSTS", rs.str())
       << "</SignedInfo>";
    ss <<       "<Signature xmlns=\"http://www.w3.org/2000/09/xmldsig#\">"
       <<         hs.str();
    ss <<         "<SignatureValue>" << createSignature(hs.str(), daToken->getBinarySecret(), "WS-SecureConversationWS-SecureConversation", nonce) << "</SignatureValue>"
       <<         "<KeyInfo>"
       <<           "<wsse:SecurityTokenReference>"
       <<             "<wsse:Reference URI=\"#SignKey\"/>"
       <<           "</wsse:SecurityTokenReference>"
       <<         "</KeyInfo>"
       <<       "</Signature>"
       <<     "</wsse:Security>"
       <<   "</s:Header>"
       <<   "<s:Body>" << rs.str() << "</s:Body>"
       << "</s:Envelope>";

    std::string rep = send("https://login.live.com/RST2.srf", ss.str());
    ss.clear();

    rapidxml::xml_document<char> doc;
    doc.parse<0>(&rep[0]);
    rapidxml::xml_node<char>* root = doc.first_node("S:Envelope");
    if (root == nullptr)
        throw std::runtime_error("Failed to get the root node");
    rapidxml::xml_node<char>* header = root->first_node("S:Header");
    rapidxml::xml_node<char>* body = root->first_node("S:Body");
    if (header == nullptr || body == nullptr)
        throw std::runtime_error("Failed to get header or body");
    rapidxml::xml_node<char>* encryptedData = body->first_node("EncryptedData");
    if (encryptedData == nullptr)
        throw std::runtime_error("Failed to find encrypted data");
    rapidxml::xml_node<char>* security = header->first_node("wsse:Security");
    if (security == nullptr)
        throw std::runtime_error("Failed to get the security node");
    std::string respNonce;
    for (auto it = security->first_node("wssc:DerivedKeyToken"); it != nullptr; it = it->next_sibling("wssc:DerivedKeyToken")) {
        rapidxml::xml_attribute<char>* attr = it->first_attribute("wsu:Id");
        rapidxml::xml_node<char>* nonceNode = it->first_node("wssc:Nonce");
        if (attr == nullptr || nonceNode == nullptr || strcmp(attr->value(), "EncKey") != 0)
            continue;
        respNonce = Base64::decode(nonceNode->value());
    }
    if (respNonce.empty())
        throw std::runtime_error("Failed to find the encryption nonce");
    std::string decrypted = decryptData(encryptedData, daToken->getBinarySecret(), respNonce);
    Log::trace("MSANetwork", "Decrypted body: %s\n", decrypted.c_str());

    rapidxml::xml_document<char> subdoc;
    subdoc.parse<0>(&decrypted[0]);
    root = subdoc.first_node("wst:RequestSecurityTokenResponseCollection");
    if (root == nullptr)
        throw std::runtime_error("Failed to get the root node");
    std::vector<MSATokenResponse> ret;
    for (auto it = root->first_node("wst:RequestSecurityTokenResponse"); it != nullptr; it = it->next_sibling("wst:RequestSecurityTokenResponse"))
        ret.push_back(parseTokenResponse(it));
    return ret;
}
