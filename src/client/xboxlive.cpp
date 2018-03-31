#include "xboxlive.h"
#include "../common/base64.h"
#include "../common/path_helper.h"
#include "../common/log.h"
#ifndef __APPLE__
#include "../ui/browser/xbox_login_browser.h"
#endif

#include <fstream>

std::shared_ptr<CLL> XboxLiveHelper::cll;
std::shared_ptr<MSALoginManager> XboxLiveHelper::msaLoginManager;
std::shared_ptr<SimpleMSAStorageManager> XboxLiveHelper::msaStorageManager;

const std::string SimpleMSAStorageManager::DEVICE_AUTH_PATH = "msa_device_auth.txt";
const std::string SimpleMSAStorageManager::ACCOUNT_INFO_PATH = "msa_account.txt";

void XboxLiveHelper::initMSALoginManager() {
    cll = std::shared_ptr<CLL>(new CLL());
    msaStorageManager = std::shared_ptr<SimpleMSAStorageManager>(new SimpleMSAStorageManager());
    msaLoginManager = std::shared_ptr<MSALoginManager>(new MSALoginManager(msaStorageManager));
}

void XboxLiveHelper::shutdown() {
    cll.reset();
}

void XboxLiveHelper::invokeXbLogin(xbox::services::system::user_auth_android* auth, std::string const& binaryToken,
                                   std::string const& cid) {
    using namespace xbox::services::system;
    auto auth_mgr = xbox::services::system::auth_manager::get_auth_manager_instance();
    auth_mgr->set_rps_ticket(binaryToken);
    auto initTask = auth_mgr->initialize_default_nsal();
    auto initRet = initTask.get();
    if (initRet.code != 0)
        throw std::runtime_error("Failed to initialize default nsal");
    std::vector<token_identity_type> types = {(token_identity_type) 3, (token_identity_type) 1,
                                              (token_identity_type) 2};
    auto config = auth_mgr->get_auth_config();
    config->set_xtoken_composition(types);
    std::string const& endpoint = config->xbox_live_endpoint().std();
    Log::trace("XboxLiveHelper", "Xbox Live Endpoint: %s", endpoint.c_str());
    auto task = auth_mgr->internal_get_token_and_signature("GET", endpoint, endpoint, std::string(), std::vector<unsigned char>(), false, false, std::string()); // I'm unsure about the vector (and pretty much only about the vector)
    Log::trace("XboxLiveHelper", "Get token and signature task started!");
    auto ret = task.get();
    Log::debug("XboxLiveHelper", "User info received! Status: %i", ret.code);
    Log::debug("XboxLiveHelper", "Gamertag = %s, age group = %s, web account id = %s\n", ret.data.gamertag.c_str(), ret.data.age_group.c_str(), ret.data.web_account_id.c_str());

    auth->auth_flow_result.xbox_user_id = ret.data.xbox_user_id;
    auth->auth_flow_result.gamertag = ret.data.gamertag;
    auth->auth_flow_result.age_group = ret.data.age_group;
    auth->auth_flow_result.privileges = ret.data.privileges;
    auth->auth_flow_result.user_settings_restrictions = ret.data.user_settings_restrictions;
    auth->auth_flow_result.user_enforcement_restrictions = ret.data.user_enforcement_restrictions;
    auth->auth_flow_result.user_title_restrictions = ret.data.user_title_restrictions;
    auth->auth_flow_result.cid = cid;
}

void XboxLiveHelper::openLoginBrowser(xbox::services::system::user_auth_android* auth) {
#ifndef __APPLE__
    auto result = XboxLoginBrowserClient::OpenBrowser();
    if (result.success) {
        XboxLiveHelper::invokeXbLogin(auth, result.binaryToken, result.cid);
        auth->auth_flow_result.code = 0;
        auth->auth_flow_result.cid = result.cid;
        auth->auth_flow_event.set(auth->auth_flow_result);
    } else {
        auth->auth_flow_result.code = 2;
        auth->auth_flow_event.set(auth->auth_flow_result);
    }
#endif
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
    std::ifstream stream (PathHelper::getPrimaryDataDirectory() + DEVICE_AUTH_PATH);
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
    std::ofstream stream (PathHelper::getPrimaryDataDirectory() + DEVICE_AUTH_PATH);
    writeProperties(stream, properties);
}

std::shared_ptr<MSAAccount> SimpleMSAStorageManager::getAccount() {
    if (account)
        return account;
    std::ifstream stream (PathHelper::getPrimaryDataDirectory() + ACCOUNT_INFO_PATH);
    if (!stream)
        return std::shared_ptr<MSAAccount>();
    std::map<std::string, std::string> properties = readProperties(stream);
    if (!properties.count("username"))
        return std::shared_ptr<MSAAccount>();
    std::shared_ptr<MSALegacyToken> token(new MSALegacyToken(properties["token_xml"], Base64::decode(properties["token_bin_secret"])));
    account = std::shared_ptr<MSAAccount>(new MSAAccount(XboxLiveHelper::getMSALoginManager(), properties["username"], properties["cid"], token));
    XboxLiveHelper::getCLL()->setMSAAccount(account);
    return account;
}

void SimpleMSAStorageManager::setAccount(std::shared_ptr<MSAAccount> account) {
    this->account = account;
    onAccountInfoChanged();
    XboxLiveHelper::getCLL()->setMSAAccount(account);
}

void SimpleMSAStorageManager::onAccountTokenListChanged(MSALoginManager& manager, MSAAccount& account) {
    if (&account != this->account.get())
        return;
    onAccountInfoChanged();
}

void SimpleMSAStorageManager::onAccountInfoChanged() {
    std::map<std::string, std::string> properties;
    if (account) {
        properties["username"] = account->getUsername();
        properties["cid"] = account->getCID();
        properties["token_xml"] = account->getDaToken()->getXmlData();
        properties["token_bin_secret"] = Base64::encode(account->getDaToken()->getBinarySecret());
        // TODO: Cached tokens
    }
    std::ofstream stream (PathHelper::getPrimaryDataDirectory() + ACCOUNT_INFO_PATH);
    writeProperties(stream, properties);
}