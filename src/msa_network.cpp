#include "msa_network.h"

#include <sstream>
#include <cstring>
#include <iostream>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <curl/curl.h>
#include "base64.h"

std::chrono::milliseconds MSANetwork::serverTimeOffset; // TODO: Update it in ::send

static size_t curl_stringstream_write_func(void* ptr, size_t size, size_t nmemb, std::stringstream* s) {
    s->write((char*) ptr, size * nmemb);
    return size * nmemb;
}

std::string MSANetwork::send(std::string const& url, std::string const& data) {
    std::cout << "Send " << url << ": " << data << "\n";

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) data.length());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Dalvik/2.1.0 (Linux; U; Android 6.0.1; ONEPLUS A3003 Build/MMB29M); com.mojang.minecraftpe/0.15.2.1; MsaAndroidSdk/2.1.0504.0524");
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    std::stringstream output;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_stringstream_write_func);
    curl_easy_perform(curl);
    std::cout << "Reply: " << output.str() << "\n";
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

std::shared_ptr<MSALegacyToken> MSANetwork::parseLegacyToken(rapidxml::xml_node<char>* node) {
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
    return std::shared_ptr<MSALegacyToken>(new MSALegacyToken(ss.str(), Base64::decode(binarySecret->value())));
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
    return parseLegacyToken(token);
}

std::vector<MSATokenResponse> MSANetwork::requestTokens(std::string const& daToken,
                                                        std::shared_ptr<MSALegacyToken> deviceToken,
                                                        std::vector<MSASecurityScope> const& scopes) {
    //
}