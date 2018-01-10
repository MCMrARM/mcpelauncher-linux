#pragma once

#include <memory>
#include <vector>
#include "string.h"

namespace xbox {
namespace services {

extern void* (*xbox_services_error_code_category)();

template <typename T>
struct xbox_live_result {

    T data;
    int code;
    void* error_code_category;
    mcpe::string message;

};

template <>
struct xbox_live_result<void> {

    int code;
    void* error_code_category;
    mcpe::string message;

};

struct java_interop {

    char filler[0xc];
    void* activity;

    static std::shared_ptr<xbox::services::java_interop> (*get_java_interop_singleton)();

};

struct local_config {

    void** vtable;

    mcpe::string get_value_from_local_storage(mcpe::string const& value) {
        mcpe::string (*func)(void*, mcpe::string const&) = (mcpe::string (*)(void*, mcpe::string const&)) vtable[0x34/4];
        return func(this, value);
    }

    static std::shared_ptr<local_config> (*local_config_get_local_config_singleton)();

};

namespace system {

struct java_rps_ticket {

    mcpe::string token;
    int error_code;
    mcpe::string error_text;

    java_rps_ticket() {}
    java_rps_ticket(java_rps_ticket const& c) {
        token = c.token;
        error_code = c.error_code;
        error_text = c.error_text;
    }

};

struct auth_flow_result {

    int code;
    mcpe::string s1, xbox_user_id, gamertag, age_group, privileges, user_settings_restrictions, user_enforcement_restrictions, user_title_restrictions, cid, s7;

    auth_flow_result() {}
    auth_flow_result(auth_flow_result const& c) {
        code = c.code;
        s1 = c.s1;
        xbox_user_id = c.xbox_user_id;
        gamertag = c.gamertag;
        age_group = c.age_group;
        privileges = c.privileges;
        user_settings_restrictions = c.user_settings_restrictions;
        user_enforcement_restrictions = c.user_enforcement_restrictions;
        user_title_restrictions = c.user_title_restrictions;
        cid = c.cid;
        s7 = c.s7;
    }

};

struct token_and_signature_result {
    mcpe::string token, signature, xbox_user_id, gamertag, xbox_user_hash, age_group, privileges, user_settings_restrictions, user_enforcement_restrictions, user_title_restrictions, web_account_id, reserved;
};

}

};
}

namespace pplx {

struct task_completion_event_java_rps_ticket {
    char filler[0xc]; // shared_ptr
    static void (*task_completion_event_java_rps_ticket_set)(task_completion_event_java_rps_ticket*, xbox::services::system::java_rps_ticket);
};
struct task_completion_event_auth_flow_result {
    char filler[0xc]; // shared_ptr
    static void (*task_completion_event_auth_flow_result_set)(task_completion_event_auth_flow_result*, xbox::services::system::auth_flow_result);
};
struct task_completion_event_xbox_live_result_void {
    char filler[0xc]; // shared_ptr
    static void (*task_completion_event_xbox_live_result_void_set)(task_completion_event_xbox_live_result_void*, xbox::services::xbox_live_result<void>);
};

struct task_impl {
    virtual ~task_impl() = 0;
};
struct task {
    std::shared_ptr<task_impl> impl;
    static xbox::services::xbox_live_result<void> (*task_xbox_live_result_void_get)(task*);
    static xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> (*task_xbox_live_result_token_and_signature_get)(task*);
};


}


struct unknown_auth_flow_class {

    // xbox::services::system::user_auth_android::auth_flow_callback
    char filler[0x58];
    pplx::task_completion_event_auth_flow_result auth_flow_event;
    char filler2[0x8c-0x58-0xc];
    xbox::services::system::auth_flow_result auth_flow_result;

};

namespace xbox {
namespace services {
namespace system {

enum class token_identity_type { };

struct auth_config {

    char filler[0x3C];
    mcpe::string* errorDetail;

    static void (*auth_config_set_xtoken_composition)(auth_config*, std::vector<xbox::services::system::token_identity_type>);
    static mcpe::string const& (*auth_config_xbox_live_endpoint)(auth_config*);

};

struct auth_manager {

    static std::shared_ptr<auth_manager> (*auth_manager_get_auth_manager_instance)();

    static void (*auth_manager_set_rps_ticket)(auth_manager*, mcpe::string const&);
    static pplx::task (*auth_manager_initialize_default_nsal)(auth_manager*);
    static pplx::task (*auth_manager_internal_get_token_and_signature)(auth_manager*, mcpe::string, mcpe::string const&, mcpe::string const&, mcpe::string, std::vector<unsigned char> const&, bool, bool, mcpe::string const&);
    static std::shared_ptr<auth_config> (*auth_manager_get_auth_config)(auth_manager*);

};

struct user_auth_android {

    static pplx::task_completion_event_java_rps_ticket* s_rpsTicketCompletionEvent;
    static pplx::task_completion_event_xbox_live_result_void* s_signOutCompleteEvent;

    static std::shared_ptr<user_auth_android> (*user_auth_android_get_instance)();

    unknown_auth_flow_class* auth_flow;
    int filler;
    mcpe::string xbox_user_id;


};

}
}
}