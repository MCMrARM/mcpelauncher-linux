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

#include "UUID.h"

mce::UUID* mce::UUID::EMPTY;

#include "ResourcePackStack.h"

void** ResourcePackStack::ResourcePackStack_vtable;
