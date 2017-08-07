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

std::string (*gl::getOpenGLVendor)();
std::string (*gl::getOpenGLRenderer)();
std::string (*gl::getOpenGLVersion)();
std::string (*gl::getOpenGLExtensions)();
void (*mce::Platform::OGL::OGL_initBindings)();

#include "Mouse.h"

void (*Mouse::feed)(char, char, short, short, short, short);

#include "Keyboard.h"

void (*Keyboard::Keyboard_feed)(unsigned char, int);
void (*Keyboard::Keyboard_feedText)(const std::string&, bool, unsigned char);
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
void (*auth_manager::auth_manager_set_rps_ticket)(auth_manager*, std::string const&);
pplx::task (*auth_manager::auth_manager_initialize_default_nsal)(auth_manager*);
std::shared_ptr<auth_config> (*auth_manager::auth_manager_get_auth_config)(auth_manager*);
pplx::task (*auth_manager::auth_manager_internal_get_token_and_signature)(auth_manager*, std::string, std::string const&, std::string const&, std::string, std::vector<unsigned char> const&, bool, bool, std::string const&);
void (*auth_config::auth_config_set_xtoken_composition)(auth_config*, std::vector<xbox::services::system::token_identity_type>);
std::string const& (*auth_config::auth_config_xbox_live_endpoint)(auth_config*);
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