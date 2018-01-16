// This file was automatically generated using tools/process_headers.py
// Generated on Tue Jan 16 2018 15:29:01 UTC

#include <hybris/dlfcn.h>
#include "../common/log.h"

#include "UserManager.h"
static std::unique_ptr<Social::UserManager> (*_Social_UserManager_CreateUserManager)();
std::unique_ptr<Social::UserManager> Social::UserManager::CreateUserManager() {
    return _Social_UserManager_CreateUserManager();
}

#include "Xbox.h"
static void * (*_xbox_services_xbox_services_error_code_category)();
void * xbox::services::xbox_services_error_code_category() {
    return _xbox_services_xbox_services_error_code_category();
}
static std::shared_ptr<xbox::services::java_interop> (*_xbox_services_java_interop_get_java_interop_singleton)();
std::shared_ptr<xbox::services::java_interop> xbox::services::java_interop::get_java_interop_singleton() {
    return _xbox_services_java_interop_get_java_interop_singleton();
}
static std::shared_ptr<xbox::services::local_config> (*_xbox_services_local_config_get_local_config_singleton)();
std::shared_ptr<xbox::services::local_config> xbox::services::local_config::get_local_config_singleton() {
    return _xbox_services_local_config_get_local_config_singleton();
}
static void (pplx::task_completion_event_java_rps_ticket::*_pplx_task_completion_event_java_rps_ticket_set)(xbox::services::system::java_rps_ticket);
void pplx::task_completion_event_java_rps_ticket::set(xbox::services::system::java_rps_ticket p1) {
    (this->*_pplx_task_completion_event_java_rps_ticket_set)(p1);
}
static void (pplx::task_completion_event_auth_flow_result::*_pplx_task_completion_event_auth_flow_result_set)(xbox::services::system::auth_flow_result);
void pplx::task_completion_event_auth_flow_result::set(xbox::services::system::auth_flow_result p1) {
    (this->*_pplx_task_completion_event_auth_flow_result_set)(p1);
}
static void (pplx::task_completion_event_xbox_live_result_void::*_pplx_task_completion_event_xbox_live_result_void_set)(xbox::services::xbox_live_result<void>);
void pplx::task_completion_event_xbox_live_result_void::set(xbox::services::xbox_live_result<void> p1) {
    (this->*_pplx_task_completion_event_xbox_live_result_void_set)(p1);
}
static xbox::services::xbox_live_result<void> (pplx::task_xbox_live_result_void::*_pplx_task_xbox_live_result_void_get)();
xbox::services::xbox_live_result<void> pplx::task_xbox_live_result_void::get() {
    return (this->*_pplx_task_xbox_live_result_void_get)();
}
static xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> (pplx::task_xbox_live_result_token_and_signature_result::*_pplx_task_xbox_live_result_token_and_signature_result_get)();
xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> pplx::task_xbox_live_result_token_and_signature_result::get() {
    return (this->*_pplx_task_xbox_live_result_token_and_signature_result_get)();
}
static void (xbox::services::system::auth_config::*_xbox_services_system_auth_config_set_xtoken_composition)(std::vector<xbox::services::system::token_identity_type>);
void xbox::services::system::auth_config::set_xtoken_composition(std::vector<xbox::services::system::token_identity_type> p1) {
    (this->*_xbox_services_system_auth_config_set_xtoken_composition)(p1);
}
static mcpe::string const & (xbox::services::system::auth_config::*_xbox_services_system_auth_config_xbox_live_endpoint)() const;
mcpe::string const & xbox::services::system::auth_config::xbox_live_endpoint() const {
    return (this->*_xbox_services_system_auth_config_xbox_live_endpoint)();
}
static std::shared_ptr<xbox::services::system::auth_manager> (*_xbox_services_system_auth_manager_get_auth_manager_instance)();
std::shared_ptr<xbox::services::system::auth_manager> xbox::services::system::auth_manager::get_auth_manager_instance() {
    return _xbox_services_system_auth_manager_get_auth_manager_instance();
}
static void (xbox::services::system::auth_manager::*_xbox_services_system_auth_manager_set_rps_ticket)(mcpe::string const &);
void xbox::services::system::auth_manager::set_rps_ticket(mcpe::string const & p1) {
    (this->*_xbox_services_system_auth_manager_set_rps_ticket)(p1);
}
static pplx::task_xbox_live_result_void (xbox::services::system::auth_manager::*_xbox_services_system_auth_manager_initialize_default_nsal)();
pplx::task_xbox_live_result_void xbox::services::system::auth_manager::initialize_default_nsal() {
    return (this->*_xbox_services_system_auth_manager_initialize_default_nsal)();
}
static pplx::task_xbox_live_result_token_and_signature_result (xbox::services::system::auth_manager::*_xbox_services_system_auth_manager_internal_get_token_and_signature)(mcpe::string, mcpe::string const &, mcpe::string const &, mcpe::string, std::vector<unsigned char> const &, bool, bool, mcpe::string const &);
pplx::task_xbox_live_result_token_and_signature_result xbox::services::system::auth_manager::internal_get_token_and_signature(mcpe::string p1, mcpe::string const & p2, mcpe::string const & p3, mcpe::string p4, std::vector<unsigned char> const & p5, bool p6, bool p7, mcpe::string const & p8) {
    return (this->*_xbox_services_system_auth_manager_internal_get_token_and_signature)(p1, p2, p3, p4, p5, p6, p7, p8);
}
static std::shared_ptr<xbox::services::system::auth_config> (xbox::services::system::auth_manager::*_xbox_services_system_auth_manager_get_auth_config)();
std::shared_ptr<xbox::services::system::auth_config> xbox::services::system::auth_manager::get_auth_config() {
    return (this->*_xbox_services_system_auth_manager_get_auth_config)();
}
pplx::task_completion_event_java_rps_ticket * xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent;
pplx::task_completion_event_xbox_live_result_void * xbox::services::system::user_auth_android::s_signOutCompleteEvent;
static std::shared_ptr<xbox::services::system::user_auth_android> (*_xbox_services_system_user_auth_android_get_instance)();
std::shared_ptr<xbox::services::system::user_auth_android> xbox::services::system::user_auth_android::get_instance() {
    return _xbox_services_system_user_auth_android_get_instance();
}

#include "ServerInstance.h"
static void (NetworkHandler::*_NetworkHandler_NetworkHandler)();
NetworkHandler::NetworkHandler() {
    (this->*_NetworkHandler_NetworkHandler)();
}
static void (ServerInstance::*_ServerInstance_ServerInstance)(IMinecraftApp &, Whitelist const &, OpsList const &, FilePathManager *, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const &, mcpe::string, LevelSettings, minecraft::api::Api &, int, bool, int, int, int, bool, std::vector<mcpe::string> const &, mcpe::string, mce::UUID const &, MinecraftEventing &, NetworkHandler &, ResourcePackRepository &, ContentTierManager const &, ResourcePackManager &, ResourcePackManager *, std::function<void ( mcpe::string const & )>);
ServerInstance::ServerInstance(IMinecraftApp & p1, Whitelist const & p2, OpsList const & p3, FilePathManager * p4, std::chrono::duration<long long> p5, mcpe::string p6, mcpe::string p7, mcpe::string p8, IContentAccessibilityProvider const & p9, mcpe::string p10, LevelSettings p11, minecraft::api::Api & p12, int p13, bool p14, int p15, int p16, int p17, bool p18, std::vector<mcpe::string> const & p19, mcpe::string p20, mce::UUID const & p21, MinecraftEventing & p22, NetworkHandler & p23, ResourcePackRepository & p24, ContentTierManager const & p25, ResourcePackManager & p26, ResourcePackManager * p27, std::function<void ( mcpe::string const & )> p28) {
    (this->*_ServerInstance_ServerInstance)(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28);
}
static void (ServerInstance::*_ServerInstance_destructor)();
ServerInstance::~ServerInstance() {
    (this->*_ServerInstance_destructor)();
}
static void (ServerInstance::*_ServerInstance_update)();
void ServerInstance::update() {
    (this->*_ServerInstance_update)();
}
static void (ServerInstance::*_ServerInstance_startLeaveGame)();
void ServerInstance::startLeaveGame() {
    (this->*_ServerInstance_startLeaveGame)();
}
static bool (ServerInstance::*_ServerInstance_isLeaveGameDone)() const;
bool ServerInstance::isLeaveGameDone() const {
    return (this->*_ServerInstance_isLeaveGameDone)();
}
static void (ServerInstance::*_ServerInstance_mainThreadNetworkUpdate_HACK)();
void ServerInstance::mainThreadNetworkUpdate_HACK() {
    (this->*_ServerInstance_mainThreadNetworkUpdate_HACK)();
}

#include "OpsList.h"
static void (OpsList::*_OpsList_OpsList)(bool);
OpsList::OpsList(bool p1) {
    (this->*_OpsList_OpsList)(p1);
}

#include "GameControllerManager.h"
GameControllerManager * GameControllerManager::sGamePadManager;
static void (GameControllerManager::*_GameControllerManager_setGameControllerConnected)(int, bool);
void GameControllerManager::setGameControllerConnected(int p1, bool p2) {
    (this->*_GameControllerManager_setGameControllerConnected)(p1, p2);
}
static void (GameControllerManager::*_GameControllerManager_feedButton)(int, int, GameControllerButtonState, bool);
void GameControllerManager::feedButton(int p1, int p2, GameControllerButtonState p3, bool p4) {
    (this->*_GameControllerManager_feedButton)(p1, p2, p3, p4);
}
static void (GameControllerManager::*_GameControllerManager_feedStick)(int, int, GameControllerStickState, float, float);
void GameControllerManager::feedStick(int p1, int p2, GameControllerStickState p3, float p4, float p5) {
    (this->*_GameControllerManager_feedStick)(p1, p2, p3, p4, p5);
}
static void (GameControllerManager::*_GameControllerManager_feedTrigger)(int, int, float);
void GameControllerManager::feedTrigger(int p1, int p2, float p3) {
    (this->*_GameControllerManager_feedTrigger)(p1, p2, p3);
}
static void (GameControllerManager::*_GameControllerManager_feedJoinGame)(int, bool);
void GameControllerManager::feedJoinGame(int p1, bool p2) {
    (this->*_GameControllerManager_feedJoinGame)(p1, p2);
}

#include "Mouse.h"
static void (*_Mouse_feed)(char, char, short, short, short, short);
void Mouse::feed(char p1, char p2, short p3, short p4, short p5, short p6) {
    _Mouse_feed(p1, p2, p3, p4, p5, p6);
}

#include "MinecraftCommands.h"
static void (MinecraftCommands::*_MinecraftCommands_setOutputSender)(std::unique_ptr<CommandOutputSender>);
void MinecraftCommands::setOutputSender(std::unique_ptr<CommandOutputSender> p1) {
    (this->*_MinecraftCommands_setOutputSender)(std::move(p1));
}
static MCRESULT (MinecraftCommands::*_MinecraftCommands_requestCommandExecution)(std::unique_ptr<CommandOrigin>, mcpe::string const &, int, bool) const;
MCRESULT MinecraftCommands::requestCommandExecution(std::unique_ptr<CommandOrigin> p1, mcpe::string const & p2, int p3, bool p4) const {
    return (this->*_MinecraftCommands_requestCommandExecution)(std::move(p1), p2, p3, p4);
}

#include "I18n.h"
static mcpe::string (*_I18n_get)(mcpe::string const &, std::vector<mcpe::string> const &);
mcpe::string I18n::get(mcpe::string const & p1, std::vector<mcpe::string> const & p2) {
    return _I18n_get(p1, p2);
}
static void (*_I18n_chooseLanguage)(mcpe::string const &);
void I18n::chooseLanguage(mcpe::string const & p1) {
    _I18n_chooseLanguage(p1);
}
static void (*_I18n_loadLanguages)(ResourcePackManager &, SkinRepository *, mcpe::string const &);
void I18n::loadLanguages(ResourcePackManager & p1, SkinRepository * p2, mcpe::string const & p3) {
    _I18n_loadLanguages(p1, p2, p3);
}

#include "gl.h"
static mcpe::string (*_gl_getOpenGLVendor)();
mcpe::string gl::getOpenGLVendor() {
    return _gl_getOpenGLVendor();
}
static mcpe::string (*_gl_getOpenGLRenderer)();
mcpe::string gl::getOpenGLRenderer() {
    return _gl_getOpenGLRenderer();
}
static mcpe::string (*_gl_getOpenGLVersion)();
mcpe::string gl::getOpenGLVersion() {
    return _gl_getOpenGLVersion();
}
static mcpe::string (*_gl_getOpenGLExtensions)();
mcpe::string gl::getOpenGLExtensions() {
    return _gl_getOpenGLExtensions();
}
static void (*_mce_Platform_OGL_InitBindings)();
void mce::Platform::OGL::InitBindings() {
    _mce_Platform_OGL_InitBindings();
}

#include "ResourcePackStack.h"
static void (PackInstance::*_PackInstance_PackInstance)(ResourcePack *, int, bool);
PackInstance::PackInstance(ResourcePack * p1, int p2, bool p3) {
    (this->*_PackInstance_PackInstance)(p1, p2, p3);
}
void * * ResourcePackStack::vtable_sym;
static void (ResourcePackStack::*_ResourcePackStack_add)(PackInstance const &, ResourcePackRepository const &, bool);
void ResourcePackStack::add(PackInstance const & p1, ResourcePackRepository const & p2, bool p3) {
    (this->*_ResourcePackStack_add)(p1, p2, p3);
}

#include "MinecraftEventing.h"
static void (MinecraftEventing::*_MinecraftEventing_MinecraftEventing)(mcpe::string const &);
MinecraftEventing::MinecraftEventing(mcpe::string const & p1) {
    (this->*_MinecraftEventing_MinecraftEventing)(p1);
}
static void (MinecraftEventing::*_MinecraftEventing_init)();
void MinecraftEventing::init() {
    (this->*_MinecraftEventing_init)();
}

#include "Api.h"

#include "MinecraftGame.h"
static void (MinecraftGame::*_MinecraftGame_MinecraftGame)(int, char * *);
MinecraftGame::MinecraftGame(int p1, char * * p2) {
    (this->*_MinecraftGame_MinecraftGame)(p1, p2);
}
static void (MinecraftGame::*_MinecraftGame_destructor)();
MinecraftGame::~MinecraftGame() {
    (this->*_MinecraftGame_destructor)();
}
static void (MinecraftGame::*_MinecraftGame_update)();
void MinecraftGame::update() {
    (this->*_MinecraftGame_update)();
}
static void (MinecraftGame::*_MinecraftGame_setRenderingSize)(int, int);
void MinecraftGame::setRenderingSize(int p1, int p2) {
    (this->*_MinecraftGame_setRenderingSize)(p1, p2);
}
static void (MinecraftGame::*_MinecraftGame_setUISizeAndScale)(int, int, float);
void MinecraftGame::setUISizeAndScale(int p1, int p2, float p3) {
    (this->*_MinecraftGame_setUISizeAndScale)(p1, p2, p3);
}
static std::shared_ptr<Options> (MinecraftGame::*_MinecraftGame_getPrimaryUserOptions)();
std::shared_ptr<Options> MinecraftGame::getPrimaryUserOptions() {
    return (this->*_MinecraftGame_getPrimaryUserOptions)();
}

#include "Keyboard.h"
int * Keyboard::_states;
static void (*_Keyboard_feed)(unsigned char, int);
void Keyboard::feed(unsigned char p1, int p2) {
    _Keyboard_feed(p1, p2);
}
static void (*_Keyboard_feedText)(mcpe::string const &, bool, unsigned char);
void Keyboard::feedText(mcpe::string const & p1, bool p2, unsigned char p3) {
    _Keyboard_feedText(p1, p2, p3);
}

#include "CommandOutput.h"
static std::vector<CommandOutputMessage> const & (CommandOutput::*_CommandOutput_getMessages)() const;
std::vector<CommandOutputMessage> const & CommandOutput::getMessages() const {
    return (this->*_CommandOutput_getMessages)();
}

#include "Scheduler.h"
static Scheduler * (*_Scheduler_singleton)();
Scheduler * Scheduler::singleton() {
    return _Scheduler_singleton();
}
static void (Scheduler::*_Scheduler_processCoroutines)(std::chrono::duration<long long>);
void Scheduler::processCoroutines(std::chrono::duration<long long> p1) {
    (this->*_Scheduler_processCoroutines)(p1);
}

#include "FilePathManager.h"
static void (FilePathManager::*_FilePathManager_FilePathManager)(mcpe::string, bool);
FilePathManager::FilePathManager(mcpe::string p1, bool p2) {
    (this->*_FilePathManager_FilePathManager)(p1, p2);
}

#include "App.h"
static void (App::*_App_init)(AppContext &);
void App::init(AppContext & p1) {
    (this->*_App_init)(p1);
}

#include "ImagePickingCallback.h"

#include "IMinecraftApp.h"

#include "Whitelist.h"

#include "AppPlatform.h"
void * * AppPlatform::myVtable;
static void (AppPlatform::*_AppPlatform_AppPlatform)();
AppPlatform::AppPlatform() {
    (this->*_AppPlatform_AppPlatform)();
}
static void (AppPlatform::*_AppPlatform__fireAppFocusGained)();
void AppPlatform::_fireAppFocusGained() {
    (this->*_AppPlatform__fireAppFocusGained)();
}
static void (AppPlatform::*_AppPlatform_initialize)();
void AppPlatform::initialize() {
    (this->*_AppPlatform_initialize)();
}

#include "CommandOutputSender.h"
static std::vector<mcpe::string> (*_CommandOutputSender_translate)(std::vector<mcpe::string> const &);
std::vector<mcpe::string> CommandOutputSender::translate(std::vector<mcpe::string> const & p1) {
    return _CommandOutputSender_translate(p1);
}
static void (CommandOutputSender::*_CommandOutputSender_CommandOutputSender)(Automation::AutomationClient &);
CommandOutputSender::CommandOutputSender(Automation::AutomationClient & p1) {
    (this->*_CommandOutputSender_CommandOutputSender)(p1);
}
static void (CommandOutputSender::*_CommandOutputSender_destructor)();
CommandOutputSender::~CommandOutputSender() {
    (this->*_CommandOutputSender_destructor)();
}
static void (CommandOutputSender::*_CommandOutputSender_send)(CommandOrigin const &, CommandOutput const &);
void CommandOutputSender::send(CommandOrigin const & p1, CommandOutput const & p2) {
    (this->*_CommandOutputSender_send)(p1, p2);
}
static void (CommandOutputSender::*_CommandOutputSender_registerOutputCallback)();
void CommandOutputSender::registerOutputCallback() {
    (this->*_CommandOutputSender_registerOutputCallback)();
}

#include "FilePickerSettings.h"

#include "AutomationClient.h"
static void (Automation::AutomationClient::*_Automation_AutomationClient_AutomationClient)(IMinecraftApp &);
Automation::AutomationClient::AutomationClient(IMinecraftApp & p1) {
    (this->*_Automation_AutomationClient_AutomationClient)(p1);
}

#include "ResourcePack.h"
static void (SkinPackKeyProvider::*_SkinPackKeyProvider_SkinPackKeyProvider)();
SkinPackKeyProvider::SkinPackKeyProvider() {
    (this->*_SkinPackKeyProvider_SkinPackKeyProvider)();
}
static void (PackManifestFactory::*_PackManifestFactory_PackManifestFactory)(IPackTelemetry &);
PackManifestFactory::PackManifestFactory(IPackTelemetry & p1) {
    (this->*_PackManifestFactory_PackManifestFactory)(p1);
}
static void (PackSourceFactory::*_PackSourceFactory_PackSourceFactory)(Options *);
PackSourceFactory::PackSourceFactory(Options * p1) {
    (this->*_PackSourceFactory_PackSourceFactory)(p1);
}
static void (ResourcePackRepository::*_ResourcePackRepository_ResourcePackRepository)(MinecraftEventing &, PackManifestFactory &, IContentAccessibilityProvider &, FilePathManager *, PackSourceFactory &);
ResourcePackRepository::ResourcePackRepository(MinecraftEventing & p1, PackManifestFactory & p2, IContentAccessibilityProvider & p3, FilePathManager * p4, PackSourceFactory & p5) {
    (this->*_ResourcePackRepository_ResourcePackRepository)(p1, p2, p3, p4, p5);
}
static void (ContentTierManager::*_ContentTierManager_ContentTierManager)();
ContentTierManager::ContentTierManager() {
    (this->*_ContentTierManager_ContentTierManager)();
}
static void (ResourcePackManager::*_ResourcePackManager_ResourcePackManager)(std::function<mcpe::string ( )> const &, ContentTierManager const &);
ResourcePackManager::ResourcePackManager(std::function<mcpe::string ( )> const & p1, ContentTierManager const & p2) {
    (this->*_ResourcePackManager_ResourcePackManager)(p1, p2);
}
static void (ResourcePackManager::*_ResourcePackManager_setStack)(std::unique_ptr<ResourcePackStack>, ResourcePackStackType, bool);
void ResourcePackManager::setStack(std::unique_ptr<ResourcePackStack> p1, ResourcePackStackType p2, bool p3) {
    (this->*_ResourcePackManager_setStack)(std::move(p1), p2, p3);
}
static void (ResourcePackManager::*_ResourcePackManager_onLanguageChanged)();
void ResourcePackManager::onLanguageChanged() {
    (this->*_ResourcePackManager_onLanguageChanged)();
}

#include "DedicatedServerCommandOrigin.h"
static void (DedicatedServerCommandOrigin::*_DedicatedServerCommandOrigin_DedicatedServerCommandOrigin)(mcpe::string const &, Minecraft &);
DedicatedServerCommandOrigin::DedicatedServerCommandOrigin(mcpe::string const & p1, Minecraft & p2) {
    (this->*_DedicatedServerCommandOrigin_DedicatedServerCommandOrigin)(p1, p2);
}

#include "MultiplayerService.h"
static void (Social::MultiplayerXBL::*_Social_MultiplayerXBL_MultiplayerXBL)();
Social::MultiplayerXBL::MultiplayerXBL() {
    (this->*_Social_MultiplayerXBL_MultiplayerXBL)();
}

#include "UUID.h"
mce::UUID * mce::UUID::EMPTY;
static mce::UUID (*_mce_UUID_fromString)(mcpe::string const &);
mce::UUID mce::UUID::fromString(mcpe::string const & p1) {
    return _mce_UUID_fromString(p1);
}

#include "Minecraft.h"
static MinecraftCommands * (Minecraft::*_Minecraft_getCommands)();
MinecraftCommands * Minecraft::getCommands() {
    return (this->*_Minecraft_getCommands)();
}

#include "LevelSettings.h"
static void (LevelSettings::*_LevelSettings_LevelSettings)();
LevelSettings::LevelSettings() {
    (this->*_LevelSettings_LevelSettings)();
}
static void (LevelSettings::*_LevelSettings_LevelSettings2)(LevelSettings const &);
LevelSettings::LevelSettings(LevelSettings const & p1) {
    (this->*_LevelSettings_LevelSettings2)(p1);
}

#include "Options.h"
static bool (Options::*_Options_getFullscreen)() const;
bool Options::getFullscreen() const {
    return (this->*_Options_getFullscreen)();
}
static void (Options::*_Options_setFullscreen)(bool);
void Options::setFullscreen(bool p1) {
    (this->*_Options_setFullscreen)(p1);
}

#include "Common.h"
static mcpe::string (*_Common_getGameVersionStringNet)();
mcpe::string Common::getGameVersionStringNet() {
    return _Common_getGameVersionStringNet();
}

void minecraft_symbols_init(void* handle) {
    ((void*&) _Social_UserManager_CreateUserManager) = hybris_dlsym(handle, "_ZN6Social11UserManager17CreateUserManagerEv");
    if (_Social_UserManager_CreateUserManager == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN6Social11UserManager17CreateUserManagerEv");
    ((void*&) _xbox_services_xbox_services_error_code_category) = hybris_dlsym(handle, "_ZN4xbox8services33xbox_services_error_code_categoryEv");
    if (_xbox_services_xbox_services_error_code_category == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services33xbox_services_error_code_categoryEv");
    ((void*&) _xbox_services_java_interop_get_java_interop_singleton) = hybris_dlsym(handle, "_ZN4xbox8services12java_interop26get_java_interop_singletonEv");
    if (_xbox_services_java_interop_get_java_interop_singleton == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services12java_interop26get_java_interop_singletonEv");
    ((void*&) _xbox_services_local_config_get_local_config_singleton) = hybris_dlsym(handle, "_ZN4xbox8services12local_config26get_local_config_singletonEv");
    if (_xbox_services_local_config_get_local_config_singleton == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services12local_config26get_local_config_singletonEv");
    ((void*&) _pplx_task_completion_event_java_rps_ticket_set) = hybris_dlsym(handle, "_ZNK4pplx21task_completion_eventIN4xbox8services6system15java_rps_ticketEE3setES4_");
    if (_pplx_task_completion_event_java_rps_ticket_set == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4pplx21task_completion_eventIN4xbox8services6system15java_rps_ticketEE3setES4_");
    ((void*&) _pplx_task_completion_event_auth_flow_result_set) = hybris_dlsym(handle, "_ZNK4pplx21task_completion_eventIN4xbox8services6system16auth_flow_resultEE3setES4_");
    if (_pplx_task_completion_event_auth_flow_result_set == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4pplx21task_completion_eventIN4xbox8services6system16auth_flow_resultEE3setES4_");
    ((void*&) _pplx_task_completion_event_xbox_live_result_void_set) = hybris_dlsym(handle, "_ZNK4pplx21task_completion_eventIN4xbox8services16xbox_live_resultIvEEE3setES4_");
    if (_pplx_task_completion_event_xbox_live_result_void_set == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4pplx21task_completion_eventIN4xbox8services16xbox_live_resultIvEEE3setES4_");
    ((void*&) _pplx_task_xbox_live_result_void_get) = hybris_dlsym(handle, "_ZNK4pplx4taskIN4xbox8services16xbox_live_resultIvEEE3getEv");
    if (_pplx_task_xbox_live_result_void_get == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4pplx4taskIN4xbox8services16xbox_live_resultIvEEE3getEv");
    ((void*&) _pplx_task_xbox_live_result_token_and_signature_result_get) = hybris_dlsym(handle, "_ZNK4pplx4taskIN4xbox8services16xbox_live_resultINS2_6system26token_and_signature_resultEEEE3getEv");
    if (_pplx_task_xbox_live_result_token_and_signature_result_get == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4pplx4taskIN4xbox8services16xbox_live_resultINS2_6system26token_and_signature_resultEEEE3getEv");
    ((void*&) _xbox_services_system_auth_config_set_xtoken_composition) = hybris_dlsym(handle, "_ZN4xbox8services6system11auth_config22set_xtoken_compositionESt6vectorINS1_19token_identity_typeESaIS4_EE");
    if (_xbox_services_system_auth_config_set_xtoken_composition == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system11auth_config22set_xtoken_compositionESt6vectorINS1_19token_identity_typeESaIS4_EE");
    ((void*&) _xbox_services_system_auth_config_xbox_live_endpoint) = hybris_dlsym(handle, "_ZNK4xbox8services6system11auth_config18xbox_live_endpointEv");
    if (_xbox_services_system_auth_config_xbox_live_endpoint == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK4xbox8services6system11auth_config18xbox_live_endpointEv");
    ((void*&) _xbox_services_system_auth_manager_get_auth_manager_instance) = hybris_dlsym(handle, "_ZN4xbox8services6system12auth_manager25get_auth_manager_instanceEv");
    if (_xbox_services_system_auth_manager_get_auth_manager_instance == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system12auth_manager25get_auth_manager_instanceEv");
    ((void*&) _xbox_services_system_auth_manager_set_rps_ticket) = hybris_dlsym(handle, "_ZN4xbox8services6system12auth_manager14set_rps_ticketERKSs");
    if (_xbox_services_system_auth_manager_set_rps_ticket == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system12auth_manager14set_rps_ticketERKSs");
    ((void*&) _xbox_services_system_auth_manager_initialize_default_nsal) = hybris_dlsym(handle, "_ZN4xbox8services6system12auth_manager23initialize_default_nsalEv");
    if (_xbox_services_system_auth_manager_initialize_default_nsal == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system12auth_manager23initialize_default_nsalEv");
    ((void*&) _xbox_services_system_auth_manager_internal_get_token_and_signature) = hybris_dlsym(handle, "_ZN4xbox8services6system12auth_manager32internal_get_token_and_signatureESsRKSsS4_SsRKSt6vectorIhSaIhEEbbS4_");
    if (_xbox_services_system_auth_manager_internal_get_token_and_signature == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system12auth_manager32internal_get_token_and_signatureESsRKSsS4_SsRKSt6vectorIhSaIhEEbbS4_");
    ((void*&) _xbox_services_system_auth_manager_get_auth_config) = hybris_dlsym(handle, "_ZN4xbox8services6system12auth_manager15get_auth_configEv");
    if (_xbox_services_system_auth_manager_get_auth_config == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system12auth_manager15get_auth_configEv");
    ((void*&) xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent) = hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android26s_rpsTicketCompletionEventE");
    if (xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system17user_auth_android26s_rpsTicketCompletionEventE");
    ((void*&) xbox::services::system::user_auth_android::s_signOutCompleteEvent) = hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android22s_signOutCompleteEventE");
    if (xbox::services::system::user_auth_android::s_signOutCompleteEvent == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system17user_auth_android22s_signOutCompleteEventE");
    ((void*&) _xbox_services_system_user_auth_android_get_instance) = hybris_dlsym(handle, "_ZN4xbox8services6system17user_auth_android12get_instanceEv");
    if (_xbox_services_system_user_auth_android_get_instance == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4xbox8services6system17user_auth_android12get_instanceEv");
    ((void*&) _NetworkHandler_NetworkHandler) = hybris_dlsym(handle, "_ZN14NetworkHandlerC2Ev");
    if (_NetworkHandler_NetworkHandler == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14NetworkHandlerC2Ev");
    ((void*&) _ServerInstance_ServerInstance) = hybris_dlsym(handle, "_ZN14ServerInstanceC2ER13IMinecraftAppRK9WhitelistRK7OpsListP15FilePathManagerNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSsSsRK19IContentKeyProviderSs13LevelSettingsRN9minecraft3api3ApiEibiiibRKSt6vectorISsSaISsEESsRKN3mce4UUIDER17MinecraftEventingR14NetworkHandlerR22ResourcePackRepositoryRK18ContentTierManagerR19ResourcePackManagerPS15_St8functionIFvRKSsEE");
    if (_ServerInstance_ServerInstance == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14ServerInstanceC2ER13IMinecraftAppRK9WhitelistRK7OpsListP15FilePathManagerNSt6chrono8durationIxSt5ratioILx1ELx1EEEESsSsSsRK19IContentKeyProviderSs13LevelSettingsRN9minecraft3api3ApiEibiiibRKSt6vectorISsSaISsEESsRKN3mce4UUIDER17MinecraftEventingR14NetworkHandlerR22ResourcePackRepositoryRK18ContentTierManagerR19ResourcePackManagerPS15_St8functionIFvRKSsEE");
    ((void*&) _ServerInstance_destructor) = hybris_dlsym(handle, "_ZN14ServerInstanceD2Ev");
    if (_ServerInstance_destructor == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14ServerInstanceD2Ev");
    ((void*&) _ServerInstance_update) = hybris_dlsym(handle, "_ZN14ServerInstance6updateEv");
    if (_ServerInstance_update == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14ServerInstance6updateEv");
    ((void*&) _ServerInstance_startLeaveGame) = hybris_dlsym(handle, "_ZN14ServerInstance14startLeaveGameEv");
    if (_ServerInstance_startLeaveGame == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14ServerInstance14startLeaveGameEv");
    ((void*&) _ServerInstance_isLeaveGameDone) = hybris_dlsym(handle, "_ZNK14ServerInstance15isLeaveGameDoneEv");
    if (_ServerInstance_isLeaveGameDone == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK14ServerInstance15isLeaveGameDoneEv");
    ((void*&) _ServerInstance_mainThreadNetworkUpdate_HACK) = hybris_dlsym(handle, "_ZN14ServerInstance28mainThreadNetworkUpdate_HACKEv");
    if (_ServerInstance_mainThreadNetworkUpdate_HACK == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN14ServerInstance28mainThreadNetworkUpdate_HACKEv");
    ((void*&) _OpsList_OpsList) = hybris_dlsym(handle, "_ZN7OpsListC2Eb");
    if (_OpsList_OpsList == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN7OpsListC2Eb");
    ((void*&) GameControllerManager::sGamePadManager) = hybris_dlsym(handle, "_ZN21GameControllerManager15sGamePadManagerE");
    if (GameControllerManager::sGamePadManager == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager15sGamePadManagerE");
    ((void*&) _GameControllerManager_setGameControllerConnected) = hybris_dlsym(handle, "_ZN21GameControllerManager26setGameControllerConnectedEib");
    if (_GameControllerManager_setGameControllerConnected == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager26setGameControllerConnectedEib");
    ((void*&) _GameControllerManager_feedButton) = hybris_dlsym(handle, "_ZN21GameControllerManager10feedButtonEii25GameControllerButtonStateb");
    if (_GameControllerManager_feedButton == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager10feedButtonEii25GameControllerButtonStateb");
    ((void*&) _GameControllerManager_feedStick) = hybris_dlsym(handle, "_ZN21GameControllerManager9feedStickEii24GameControllerStickStateff");
    if (_GameControllerManager_feedStick == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager9feedStickEii24GameControllerStickStateff");
    ((void*&) _GameControllerManager_feedTrigger) = hybris_dlsym(handle, "_ZN21GameControllerManager11feedTriggerEiif");
    if (_GameControllerManager_feedTrigger == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager11feedTriggerEiif");
    ((void*&) _GameControllerManager_feedJoinGame) = hybris_dlsym(handle, "_ZN21GameControllerManager12feedJoinGameEib");
    if (_GameControllerManager_feedJoinGame == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN21GameControllerManager12feedJoinGameEib");
    ((void*&) _Mouse_feed) = hybris_dlsym(handle, "_ZN5Mouse4feedEccssss");
    if (_Mouse_feed == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN5Mouse4feedEccssss");
    ((void*&) _MinecraftCommands_setOutputSender) = hybris_dlsym(handle, "_ZN17MinecraftCommands15setOutputSenderESt10unique_ptrI19CommandOutputSenderSt14default_deleteIS1_EE");
    if (_MinecraftCommands_setOutputSender == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN17MinecraftCommands15setOutputSenderESt10unique_ptrI19CommandOutputSenderSt14default_deleteIS1_EE");
    ((void*&) _MinecraftCommands_requestCommandExecution) = hybris_dlsym(handle, "_ZNK17MinecraftCommands23requestCommandExecutionESt10unique_ptrI13CommandOriginSt14default_deleteIS1_EERKSsib");
    if (_MinecraftCommands_requestCommandExecution == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK17MinecraftCommands23requestCommandExecutionESt10unique_ptrI13CommandOriginSt14default_deleteIS1_EERKSsib");
    ((void*&) _I18n_get) = hybris_dlsym(handle, "_ZN4I18n3getERKSsRKSt6vectorISsSaISsEE");
    if (_I18n_get == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4I18n3getERKSsRKSt6vectorISsSaISsEE");
    ((void*&) _I18n_chooseLanguage) = hybris_dlsym(handle, "_ZN4I18n14chooseLanguageERKSs");
    if (_I18n_chooseLanguage == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4I18n14chooseLanguageERKSs");
    ((void*&) _I18n_loadLanguages) = hybris_dlsym(handle, "_ZN4I18n13loadLanguagesER19ResourcePackManagerP14SkinRepositoryRKSs");
    if (_I18n_loadLanguages == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN4I18n13loadLanguagesER19ResourcePackManagerP14SkinRepositoryRKSs");
    ((void*&) _gl_getOpenGLVendor) = hybris_dlsym(handle, "_ZN2gl15getOpenGLVendorEv");
    if (_gl_getOpenGLVendor == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN2gl15getOpenGLVendorEv");
    ((void*&) _gl_getOpenGLRenderer) = hybris_dlsym(handle, "_ZN2gl17getOpenGLRendererEv");
    if (_gl_getOpenGLRenderer == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN2gl17getOpenGLRendererEv");
    ((void*&) _gl_getOpenGLVersion) = hybris_dlsym(handle, "_ZN2gl16getOpenGLVersionEv");
    if (_gl_getOpenGLVersion == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN2gl16getOpenGLVersionEv");
    ((void*&) _gl_getOpenGLExtensions) = hybris_dlsym(handle, "_ZN2gl19getOpenGLExtensionsEv");
    if (_gl_getOpenGLExtensions == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN2gl19getOpenGLExtensionsEv");
    ((void*&) _mce_Platform_OGL_InitBindings) = hybris_dlsym(handle, "_ZN3mce8Platform3OGL12InitBindingsEv");
    if (_mce_Platform_OGL_InitBindings == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN3mce8Platform3OGL12InitBindingsEv");
    ((void*&) _PackInstance_PackInstance) = hybris_dlsym(handle, "_ZN12PackInstanceC2EP12ResourcePackib");
    if (_PackInstance_PackInstance == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN12PackInstanceC2EP12ResourcePackib");
    ((void*&) ResourcePackStack::vtable_sym) = hybris_dlsym(handle, "_ZTV17ResourcePackStack");
    if (ResourcePackStack::vtable_sym == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZTV17ResourcePackStack");
    ((void*&) _ResourcePackStack_add) = hybris_dlsym(handle, "_ZN17ResourcePackStack3addE12PackInstanceRK22ResourcePackRepositoryb");
    if (_ResourcePackStack_add == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN17ResourcePackStack3addE12PackInstanceRK22ResourcePackRepositoryb");
    ((void*&) _MinecraftEventing_MinecraftEventing) = hybris_dlsym(handle, "_ZN17MinecraftEventingC2ERKSs");
    if (_MinecraftEventing_MinecraftEventing == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN17MinecraftEventingC2ERKSs");
    ((void*&) _MinecraftEventing_init) = hybris_dlsym(handle, "_ZN17MinecraftEventing4initEv");
    if (_MinecraftEventing_init == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN17MinecraftEventing4initEv");
    ((void*&) _MinecraftGame_MinecraftGame) = hybris_dlsym(handle, "_ZN13MinecraftGameC2EiPPc");
    if (_MinecraftGame_MinecraftGame == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGameC2EiPPc");
    ((void*&) _MinecraftGame_destructor) = hybris_dlsym(handle, "_ZN13MinecraftGameD2Ev");
    if (_MinecraftGame_destructor == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGameD2Ev");
    ((void*&) _MinecraftGame_update) = hybris_dlsym(handle, "_ZN13MinecraftGame6updateEv");
    if (_MinecraftGame_update == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGame6updateEv");
    ((void*&) _MinecraftGame_setRenderingSize) = hybris_dlsym(handle, "_ZN13MinecraftGame16setRenderingSizeEii");
    if (_MinecraftGame_setRenderingSize == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGame16setRenderingSizeEii");
    ((void*&) _MinecraftGame_setUISizeAndScale) = hybris_dlsym(handle, "_ZN13MinecraftGame17setUISizeAndScaleEiif");
    if (_MinecraftGame_setUISizeAndScale == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGame17setUISizeAndScaleEiif");
    ((void*&) _MinecraftGame_getPrimaryUserOptions) = hybris_dlsym(handle, "_ZN13MinecraftGame21getPrimaryUserOptionsEv");
    if (_MinecraftGame_getPrimaryUserOptions == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13MinecraftGame21getPrimaryUserOptionsEv");
    ((void*&) Keyboard::_states) = hybris_dlsym(handle, "_ZN8Keyboard7_statesE");
    if (Keyboard::_states == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN8Keyboard7_statesE");
    ((void*&) _Keyboard_feed) = hybris_dlsym(handle, "_ZN8Keyboard4feedEhi");
    if (_Keyboard_feed == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN8Keyboard4feedEhi");
    ((void*&) _Keyboard_feedText) = hybris_dlsym(handle, "_ZN8Keyboard8feedTextERKSsbh");
    if (_Keyboard_feedText == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN8Keyboard8feedTextERKSsbh");
    ((void*&) _CommandOutput_getMessages) = hybris_dlsym(handle, "_ZNK13CommandOutput11getMessagesEv");
    if (_CommandOutput_getMessages == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK13CommandOutput11getMessagesEv");
    ((void*&) _Scheduler_singleton) = hybris_dlsym(handle, "_ZN9Scheduler9singletonEv");
    if (_Scheduler_singleton == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN9Scheduler9singletonEv");
    ((void*&) _Scheduler_processCoroutines) = hybris_dlsym(handle, "_ZN9Scheduler17processCoroutinesENSt6chrono8durationIxSt5ratioILx1ELx1000000000EEEE");
    if (_Scheduler_processCoroutines == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN9Scheduler17processCoroutinesENSt6chrono8durationIxSt5ratioILx1ELx1000000000EEEE");
    ((void*&) _FilePathManager_FilePathManager) = hybris_dlsym(handle, "_ZN15FilePathManagerC2ESsb");
    if (_FilePathManager_FilePathManager == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN15FilePathManagerC2ESsb");
    ((void*&) _App_init) = hybris_dlsym(handle, "_ZN3App4initER10AppContext");
    if (_App_init == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN3App4initER10AppContext");
    ((void*&) AppPlatform::myVtable) = hybris_dlsym(handle, "_ZTV11AppPlatform");
    if (AppPlatform::myVtable == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZTV11AppPlatform");
    ((void*&) _AppPlatform_AppPlatform) = hybris_dlsym(handle, "_ZN11AppPlatformC2Ev");
    if (_AppPlatform_AppPlatform == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN11AppPlatformC2Ev");
    ((void*&) _AppPlatform__fireAppFocusGained) = hybris_dlsym(handle, "_ZN11AppPlatform19_fireAppFocusGainedEv");
    if (_AppPlatform__fireAppFocusGained == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN11AppPlatform19_fireAppFocusGainedEv");
    ((void*&) _AppPlatform_initialize) = hybris_dlsym(handle, "_ZN11AppPlatform10initializeEv");
    if (_AppPlatform_initialize == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN11AppPlatform10initializeEv");
    ((void*&) _CommandOutputSender_translate) = hybris_dlsym(handle, "_ZN19CommandOutputSender9translateERKSt6vectorISsSaISsEE");
    if (_CommandOutputSender_translate == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19CommandOutputSender9translateERKSt6vectorISsSaISsEE");
    ((void*&) _CommandOutputSender_CommandOutputSender) = hybris_dlsym(handle, "_ZN19CommandOutputSenderC2ERN10Automation16AutomationClientE");
    if (_CommandOutputSender_CommandOutputSender == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19CommandOutputSenderC2ERN10Automation16AutomationClientE");
    ((void*&) _CommandOutputSender_destructor) = hybris_dlsym(handle, "_ZN19CommandOutputSenderD2Ev");
    if (_CommandOutputSender_destructor == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19CommandOutputSenderD2Ev");
    ((void*&) _CommandOutputSender_send) = hybris_dlsym(handle, "_ZN19CommandOutputSender4sendERK13CommandOriginRK13CommandOutput");
    if (_CommandOutputSender_send == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19CommandOutputSender4sendERK13CommandOriginRK13CommandOutput");
    ((void*&) _CommandOutputSender_registerOutputCallback) = hybris_dlsym(handle, "_ZN19CommandOutputSender22registerOutputCallbackERKSt8functionIFvR19AutomationCmdOutputEE");
    if (_CommandOutputSender_registerOutputCallback == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19CommandOutputSender22registerOutputCallbackERKSt8functionIFvR19AutomationCmdOutputEE");
    ((void*&) _Automation_AutomationClient_AutomationClient) = hybris_dlsym(handle, "_ZN10Automation16AutomationClientC2ER13IMinecraftApp");
    if (_Automation_AutomationClient_AutomationClient == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN10Automation16AutomationClientC2ER13IMinecraftApp");
    ((void*&) _SkinPackKeyProvider_SkinPackKeyProvider) = hybris_dlsym(handle, "_ZN19SkinPackKeyProviderC2Ev");
    if (_SkinPackKeyProvider_SkinPackKeyProvider == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19SkinPackKeyProviderC2Ev");
    ((void*&) _PackManifestFactory_PackManifestFactory) = hybris_dlsym(handle, "_ZN19PackManifestFactoryC2ER14IPackTelemetry");
    if (_PackManifestFactory_PackManifestFactory == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19PackManifestFactoryC2ER14IPackTelemetry");
    ((void*&) _PackSourceFactory_PackSourceFactory) = hybris_dlsym(handle, "_ZN17PackSourceFactoryC2EP7Options");
    if (_PackSourceFactory_PackSourceFactory == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN17PackSourceFactoryC2EP7Options");
    ((void*&) _ResourcePackRepository_ResourcePackRepository) = hybris_dlsym(handle, "_ZN22ResourcePackRepositoryC2ER17MinecraftEventingR19PackManifestFactoryR29IContentAccessibilityProviderP15FilePathManagerR17PackSourceFactory");
    if (_ResourcePackRepository_ResourcePackRepository == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN22ResourcePackRepositoryC2ER17MinecraftEventingR19PackManifestFactoryR29IContentAccessibilityProviderP15FilePathManagerR17PackSourceFactory");
    ((void*&) _ContentTierManager_ContentTierManager) = hybris_dlsym(handle, "_ZN18ContentTierManagerC2Ev");
    if (_ContentTierManager_ContentTierManager == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN18ContentTierManagerC2Ev");
    ((void*&) _ResourcePackManager_ResourcePackManager) = hybris_dlsym(handle, "_ZN19ResourcePackManagerC2ESt8functionIFSsvEERK18ContentTierManager");
    if (_ResourcePackManager_ResourcePackManager == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19ResourcePackManagerC2ESt8functionIFSsvEERK18ContentTierManager");
    ((void*&) _ResourcePackManager_setStack) = hybris_dlsym(handle, "_ZN19ResourcePackManager8setStackESt10unique_ptrI17ResourcePackStackSt14default_deleteIS1_EE21ResourcePackStackTypeb");
    if (_ResourcePackManager_setStack == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19ResourcePackManager8setStackESt10unique_ptrI17ResourcePackStackSt14default_deleteIS1_EE21ResourcePackStackTypeb");
    ((void*&) _ResourcePackManager_onLanguageChanged) = hybris_dlsym(handle, "_ZN19ResourcePackManager17onLanguageChangedEv");
    if (_ResourcePackManager_onLanguageChanged == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN19ResourcePackManager17onLanguageChangedEv");
    ((void*&) _DedicatedServerCommandOrigin_DedicatedServerCommandOrigin) = hybris_dlsym(handle, "_ZN28DedicatedServerCommandOriginC2ERKSsR9Minecraft");
    if (_DedicatedServerCommandOrigin_DedicatedServerCommandOrigin == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN28DedicatedServerCommandOriginC2ERKSsR9Minecraft");
    ((void*&) _Social_MultiplayerXBL_MultiplayerXBL) = hybris_dlsym(handle, "_ZN6Social14MultiplayerXBLC2Ev");
    if (_Social_MultiplayerXBL_MultiplayerXBL == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN6Social14MultiplayerXBLC2Ev");
    ((void*&) mce::UUID::EMPTY) = hybris_dlsym(handle, "_ZN3mce4UUID5EMPTYE");
    if (mce::UUID::EMPTY == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN3mce4UUID5EMPTYE");
    ((void*&) _mce_UUID_fromString) = hybris_dlsym(handle, "_ZN3mce4UUID10fromStringERKSs");
    if (_mce_UUID_fromString == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN3mce4UUID10fromStringERKSs");
    ((void*&) _Minecraft_getCommands) = hybris_dlsym(handle, "_ZN9Minecraft11getCommandsEv");
    if (_Minecraft_getCommands == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN9Minecraft11getCommandsEv");
    ((void*&) _LevelSettings_LevelSettings) = hybris_dlsym(handle, "_ZN13LevelSettingsC2Ev");
    if (_LevelSettings_LevelSettings == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13LevelSettingsC2Ev");
    ((void*&) _LevelSettings_LevelSettings2) = hybris_dlsym(handle, "_ZN13LevelSettingsC2ERKS_");
    if (_LevelSettings_LevelSettings2 == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN13LevelSettingsC2ERKS_");
    ((void*&) _Options_getFullscreen) = hybris_dlsym(handle, "_ZNK7Options13getFullscreenEv");
    if (_Options_getFullscreen == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZNK7Options13getFullscreenEv");
    ((void*&) _Options_setFullscreen) = hybris_dlsym(handle, "_ZN7Options13setFullscreenEb");
    if (_Options_setFullscreen == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN7Options13setFullscreenEb");
    ((void*&) _Common_getGameVersionStringNet) = hybris_dlsym(handle, "_ZN6Common23getGameVersionStringNetEv");
    if (_Common_getGameVersionStringNet == nullptr) Log::error("MinecraftSymbols", "Unresolved symbol: %s", "_ZN6Common23getGameVersionStringNetEv");
}
