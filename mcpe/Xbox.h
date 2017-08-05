#pragma once

#include <string>
#include <memory>
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

    int i;
    mcpe::string s1, s2, s3, s4, s5, s6, s7;

    auth_flow_result() {}
    auth_flow_result(auth_flow_result const& c) {
        i = c.i;
        s1 = c.s1;
        s2 = c.s2;
        s3 = c.s3;
        s4 = c.s4;
        s5 = c.s5;
        s6 = c.s6;
        s7 = c.s7;
    }

};

struct token_and_signature_result {
    mcpe::string token, signature, xbox_user_id, gamertag, xbox_user_hash, age_group, priviledges, web_account_id, reserved;
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
    virtual ~task_impl();
};
struct task {
    std::shared_ptr<task_impl> impl;
    static xbox::services::xbox_live_result<void> (*task_xbox_live_result_void_get)(task*);
    static xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> (*task_xbox_live_result_token_and_signature_get)(task*);
};


}


struct unknown_auth_flow_class {

    char filler[0x48];
    pplx::task_completion_event_auth_flow_result auth_flow_event;
    char filler2[0x74-0x48-0xc];
    xbox::services::system::auth_flow_result auth_flow_result;

};

namespace xbox {
namespace services {
namespace system {

enum class token_identity_type { };

struct auth_config {

    static void (*auth_config_set_xtoken_composition)(auth_config*, std::vector<xbox::services::system::token_identity_type>);
    static std::string const& (*auth_config_xbox_live_endpoint)(auth_config*);

};

struct auth_manager {


    static void (*auth_manager_set_rps_ticket)(auth_manager*, std::string const&);
    static pplx::task (*auth_manager_initialize_default_nsal)(auth_manager*);
    static pplx::task (*auth_manager_internal_get_token_and_signature)(auth_manager*, std::string, std::string const&, std::string const&, std::string, std::vector<unsigned char> const&, bool, bool, std::string const&);
    static std::shared_ptr<auth_config> (*auth_manager_get_auth_config)(auth_manager*);

};

struct user_auth_android {

    static pplx::task_completion_event_java_rps_ticket* s_rpsTicketCompletionEvent;
    static pplx::task_completion_event_xbox_live_result_void* s_signOutCompleteEvent;

    unknown_auth_flow_class* auth_flow;
    char filler[0x60 - 4];
    auth_manager* auth_mgr;


};

}
}
}