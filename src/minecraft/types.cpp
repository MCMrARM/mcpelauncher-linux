#include <string>
#include <vector>

#include "AppPlatform.h"

AppPlatform** AppPlatform::_singleton = nullptr;
void** AppPlatform::myVtable = nullptr;

#include "MinecraftGame.h"

void (*MinecraftGame::MinecraftGame_destruct)(MinecraftGame*);

#include "Keyboard.h"

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

#include "ResourcePack.h"

void (*ResourcePackManager::ResourcePackManager_construct)(ResourcePackManager*, std::function<mcpe::string ()> const&, ContentTierManager const&);

#include "UUID.h"

mce::UUID* mce::UUID::EMPTY;

#include "ServerInstance.h"

void (*ServerInstance::ServerInstance_construct)(ServerInstance*, IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, NetworkHandler&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>);

#include "Scheduler.h"

void (*Scheduler::Scheduler_processCoroutines)(Scheduler*, std::chrono::duration<long long>);

#include "CommandOutputSender.h"

void (*CommandOutputSender::CommandOutputSender_destruct)(CommandOutputSender*);

#include "ResourcePackStack.h"

void** ResourcePackStack::ResourcePackStack_vtable;
void (*ResourcePackStack::ResourcePackStack_add)(ResourcePackStack*, PackInstance const&, ResourcePackRepository const&, bool);
