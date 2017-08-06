#include "xboxlive.h"
#include "base64.h"

#include <fstream>

std::shared_ptr<MSALoginManager> XboxLiveHelper::msaLoginManager;
std::shared_ptr<SimpleMSAStorageManager> XboxLiveHelper::msaStorageManager;

const std::string SimpleMSAStorageManager::DEVICE_AUTH_PATH = "data/msa_device_auth.txt";
const std::string SimpleMSAStorageManager::ACCOUNT_INFO_PATH = "data/msa_account.txt";

void XboxLiveHelper::initMSALoginManager() {
    msaStorageManager = std::shared_ptr<SimpleMSAStorageManager>(new SimpleMSAStorageManager());
    msaLoginManager = std::shared_ptr<MSALoginManager>(new MSALoginManager(msaStorageManager));
}

std::map<std::string, std::string> SimpleMSAStorageManager::readProperties(std::istream& stream) {
    std::string line;
    std::map<std::string, std::string> map;
    while (std::getline(stream, line)) {
        auto iof = line.find('=');
        if (iof == std::string::npos)
            continue;
        map[line.substr(0, iof)] = line.substr(iof + 1);
    }
    return map;
}

void SimpleMSAStorageManager::writeProperties(std::ostream& stream,
                                              std::map<std::string, std::string> const& properties) {
    for (auto const& prop : properties) {
        if (prop.first.find('=') != std::string::npos)
            throw std::runtime_error("Invalid key");
        if (prop.second.find('\n') != std::string::npos)
            throw std::runtime_error("Invalid value");
        stream << prop.first << "=" << prop.second << "\n";
    }
}

void SimpleMSAStorageManager::readDeviceAuthInfo(MSALoginManager& manager, MSADeviceAuth& deviceAuth) {
    std::ifstream stream (DEVICE_AUTH_PATH);
    if (!stream)
        return;
    std::map<std::string, std::string> properties = readProperties(stream);
    deviceAuth.membername = properties["membername"];
    deviceAuth.password = properties["password"];
    deviceAuth.puid = properties["puid"];
    if (properties.count("token_xml") > 0 && properties.count("token_bin_secret") > 0)
        deviceAuth.token = std::shared_ptr<MSALegacyToken>(new MSALegacyToken(properties["token_xml"], Base64::decode(properties["token_bin_secret"])));
}

void SimpleMSAStorageManager::onDeviceAuthChanged(MSALoginManager& manager, MSADeviceAuth& deviceAuth) {
    std::map<std::string, std::string> properties;
    properties["membername"] = deviceAuth.membername;
    properties["password"] = deviceAuth.password;
    properties["puid"] = deviceAuth.puid;
    if (deviceAuth.token) {
        properties["token_xml"] = deviceAuth.token->getXmlData();
        properties["token_bin_secret"] = Base64::encode(deviceAuth.token->getBinarySecret());
    }
    std::ofstream stream (DEVICE_AUTH_PATH);
    writeProperties(stream, properties);
}

std::shared_ptr<MSAAccount> SimpleMSAStorageManager::getAccount() {
    if (account)
        return account;
    std::ifstream stream (ACCOUNT_INFO_PATH);
    if (!stream)
        return std::shared_ptr<MSAAccount>();
    std::map<std::string, std::string> properties = readProperties(stream);
    std::shared_ptr<MSALegacyToken> token(new MSALegacyToken(properties["token_xml"], Base64::decode(properties["token_bin_secret"])));
    account = std::shared_ptr<MSAAccount>(new MSAAccount(XboxLiveHelper::getMSALoginManager(), properties["username"], properties["cid"], token));
    if (account->getUsername().empty()) // this shouldn't happen, but still
        return std::shared_ptr<MSAAccount>();
    return account;
}

void SimpleMSAStorageManager::onAccountTokenListChanged(MSALoginManager& manager, MSAAccount& account) {
    if (&account != this->account.get())
        return;
    onAccountInfoChanged();
}

void SimpleMSAStorageManager::onAccountInfoChanged() {
    std::map<std::string, std::string> properties;
    properties["username"] = account->getUsername();
    properties["cid"] = account->getCID();
    properties["token_xml"] = account->getDaToken()->getXmlData();
    properties["token_bin_secret"] = Base64::encode(account->getDaToken()->getBinarySecret());
    // TODO: Cached tokens
    std::ofstream stream (ACCOUNT_INFO_PATH);
    writeProperties(stream, properties);
}