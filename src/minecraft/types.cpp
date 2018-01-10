#include <string>
#include <vector>

#include "AppPlatform.h"

AppPlatform** AppPlatform::_singleton = nullptr;
void** AppPlatform::myVtable = nullptr;

#include "Keyboard.h"

int* Keyboard::states;

#include "Xbox.h"

namespace xbox {
namespace services {
void* (*xbox_services_error_code_category)();
namespace system {
pplx::task_completion_event_java_rps_ticket* xbox::services::system::user_auth_android::s_rpsTicketCompletionEvent;
pplx::task_completion_event_xbox_live_result_void* xbox::services::system::user_auth_android::s_signOutCompleteEvent;
}
}
}

#include "ResourcePack.h"

void (*ResourcePackManager::ResourcePackManager_construct)(ResourcePackManager*, std::function<mcpe::string ()> const&, ContentTierManager const&);

#include "UUID.h"

mce::UUID* mce::UUID::EMPTY;

#include "ServerInstance.h"

void (*ServerInstance::ServerInstance_construct)(ServerInstance*, IMinecraftApp&, Whitelist const&, OpsList const&, FilePathManager*, std::chrono::duration<long long>, mcpe::string, mcpe::string, mcpe::string, IContentAccessibilityProvider const&, mcpe::string, LevelSettings, minecraft::api::Api&, int, bool, int, int, int, bool, std::vector<mcpe::string> const&, mcpe::string, mce::UUID const&, MinecraftEventing&, NetworkHandler&, ResourcePackRepository&, ContentTierManager const&, ResourcePackManager&, ResourcePackManager*, std::function<void (mcpe::string const&)>);

#include "Scheduler.h"

void (*Scheduler::Scheduler_processCoroutines)(Scheduler*, std::chrono::duration<long long>);

#include "ResourcePackStack.h"

void** ResourcePackStack::ResourcePackStack_vtable;
void (*ResourcePackStack::ResourcePackStack_add)(ResourcePackStack*, PackInstance const&, ResourcePackRepository const&, bool);
