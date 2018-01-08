#include <string>
#include <vector>

#include "AppPlatform.h"

AppPlatform** AppPlatform::_singleton = nullptr;
void** AppPlatform::myVtable = nullptr;
void (*AppPlatform::AppPlatform_construct)(AppPlatform*);
void (*AppPlatform::AppPlatform__fireAppFocusGained)(AppPlatform*);
void (*AppPlatform::AppPlatform_initialize)(AppPlatform*);

#include "App.h"

void (*App::App_init)(App*, AppContext&);

#include "MinecraftGame.h"

void (*MinecraftGame::MinecraftGame_construct)(MinecraftGame*, int, char**);
void (*MinecraftGame::MinecraftGame_destruct)(MinecraftGame*);
void (*MinecraftGame::MinecraftGame_update)(MinecraftGame*);
void (*MinecraftGame::MinecraftGame_setRenderingSize)(MinecraftGame*, int, int);
void (*MinecraftGame::MinecraftGame_setUISizeAndScale)(MinecraftGame*, int, int, float);
std::shared_ptr<Options> (*MinecraftGame::MinecraftGame_getPrimaryUserOptions)(MinecraftGame*);

#include "Options.h"

bool (*Options::Options_getFullscreen)(Options*);
void (*Options::Options_setFullscreen)(Options*, bool);

#include "gl.h"

mcpe::string (*gl::getOpenGLVendor)();
mcpe::string (*gl::getOpenGLRenderer)();
mcpe::string (*gl::getOpenGLVersion)();
mcpe::string (*gl::getOpenGLExtensions)();
void (*mce::Platform::OGL::OGL_initBindings)();

#include "Mouse.h"

void (*Mouse::feed)(char, char, short, short, short, short);

#include "Keyboard.h"

void (*Keyboard::Keyboard_feed)(unsigned char, int);
void (*Keyboard::Keyboard_feedText)(const mcpe::string&, bool, unsigned char);
int* Keyboard::states;

#include "Xbox.h"

namespace xbox {
namespace services {
void* (*xbox_services_error_code_category)();
std::shared_ptr<xbox::services::java_interop> (*java_interop::get_java_interop_singleton)();
std::shared_ptr<local_config> (*local_config::local_config_get_local_config_singleton)();
namespace system {
pplx::task_completion_event_java_rps_ticket* xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent;
pplx::task_completion_event_xbox_live_result_void* xbox::services::system::user_auth_android::s_signOutCompleteEvent;
std::shared_ptr<user_auth_android> (*user_auth_android::user_auth_android_get_instance)();
std::shared_ptr<auth_manager> (*auth_manager::auth_manager_get_auth_manager_instance)();
void (*auth_manager::auth_manager_set_rps_ticket)(auth_manager*, mcpe::string const&);
pplx::task (*auth_manager::auth_manager_initialize_default_nsal)(auth_manager*);
std::shared_ptr<auth_config> (*auth_manager::auth_manager_get_auth_config)(auth_manager*);
pplx::task (*auth_manager::auth_manager_internal_get_token_and_signature)(auth_manager*, mcpe::string, mcpe::string const&, mcpe::string const&, mcpe::string, std::vector<unsigned char> const&, bool, bool, mcpe::string const&);
void (*auth_config::auth_config_set_xtoken_composition)(auth_config*, std::vector<xbox::services::system::token_identity_type>);
mcpe::string const& (*auth_config::auth_config_xbox_live_endpoint)(auth_config*);
}
}
}
namespace pplx {
void (*task_completion_event_java_rps_ticket::task_completion_event_java_rps_ticket_set)(task_completion_event_java_rps_ticket*, xbox::services::system::java_rps_ticket);
void (*task_completion_event_auth_flow_result::task_completion_event_auth_flow_result_set)(task_completion_event_auth_flow_result*, xbox::services::system::auth_flow_result);
void (*task_completion_event_xbox_live_result_void::task_completion_event_xbox_live_result_void_set)(task_completion_event_xbox_live_result_void*, xbox::services::xbox_live_result<void>);
xbox::services::xbox_live_result<void> (*task::task_xbox_live_result_void_get)(task*);
xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> (*task::task_xbox_live_result_token_and_signature_get)(task*);
}

#include "MultiplayerService.h"

void (*Social::MultiplayerXBL::MultiplayerXBL_MultiplayerXBL)(Social::MultiplayerXBL*);

#include "Common.h"

mcpe::string (*Common::Common_getGameVersionStringNet)();

#include "LevelSettings.h"

void (*LevelSettings::LevelSettings_construct)(LevelSettings*);
void (*LevelSettings::LevelSettings_construct2)(LevelSettings*, LevelSettings const&);

#include "MinecraftEventing.h"

void (*MinecraftEventing::MinecraftEventing_construct)(MinecraftEventing*, mcpe::string const&);
void (*MinecraftEventing::MinecraftEventing_init)(MinecraftEventing*);

#include "ResourcePack.h"

void (*SkinPackKeyProvider::SkinPackKeyProvider_construct)(SkinPackKeyProvider*);
void (*PackManifestFactory::PackManifestFactory_construct)(PackManifestFactory*, IPackTelemetry&);
void (*PackSourceFactory::PackSourceFactory_construct)(PackSourceFactory*, Options*);
void (*ContentTierManager::ContentTierManager_construct)(ContentTierManager*);
void (*ResourcePackRepository::ResourcePackRepository_construct)(ResourcePackRepository*, MinecraftEventing&, PackManifestFactory&, IContentAccessibilityProvider&, FilePathManager*, PackSourceFactory &);
void (*ResourcePackManager::ResourcePackManager_construct)(ResourcePackManager*, std::function<mcpe::string ()> const&, ContentTierManager const&);

#include "FilePathManager.h"

void (*FilePathManager::FilePathManager_construct)(FilePathManager*, mcpe::string, bool);

#include "UUID.h"

mce::UUID* mce::UUID::EMPTY;
mce::UUID (*mce::UUID::fromString)(mcpe::string const&);

#include "ServerInstance.h"

void (*NetworkHandler::NetworkHandler_construct)(NetworkHandler*);
void (*ServerInstance::ServerInstance_construct)(ServerInstance*, IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, NetworkHandler&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>);
void (*ServerInstance::ServerInstance_update)(ServerInstance*);
void (*ServerInstance::ServerInstance_mainThreadNetworkUpdate_HACK)(ServerInstance*);

#include "UserManager.h"

std::unique_ptr<Social::UserManager> (*Social::UserManager::CreateUserManager)();

#include "AutomationClient.h"

void (*Automation::AutomationClient::AutomationClient_construct)(Automation::AutomationClient*, IMinecraftApp&);

#include "Scheduler.h"

Scheduler* (*Scheduler::singleton)();
void (*Scheduler::Scheduler_processCoroutines)(Scheduler*, std::chrono::duration<long long>);

#include "Minecraft.h"

MinecraftCommands* (*Minecraft::Minecraft_getCommands)(Minecraft*);

#include "MinecraftCommands.h"

MCRESULT (*MinecraftCommands::MinecraftCommands_requestCommandExecution)(MinecraftCommands*, std::unique_ptr<CommandOrigin>, mcpe::string const&, int, bool);

#include "DedicatedServerCommandOrigin.h"

void (*DedicatedServerCommandOrigin::DedicatedServerCommandOrigin_construct)(DedicatedServerCommandOrigin*, mcpe::string const&, Minecraft&);