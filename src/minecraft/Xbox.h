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

    static std::shared_ptr<xbox::services::java_interop> get_java_interop_singleton();

};

struct local_config {

    void** vtable;

    mcpe::string get_value_from_local_storage(mcpe::string const& value) {
        mcpe::string (*func)(void*, mcpe::string const&) = (mcpe::string (*)(void*, mcpe::string const&)) vtable[0x34/4];
        return func(this, value);
    }

    static std::shared_ptr<xbox::services::local_config> get_local_config_singleton();

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

    /// @symbol _ZNK4pplx21task_completion_eventIN4xbox8services6system15java_rps_ticketEE3setES4_
    void set(xbox::services::system::java_rps_ticket);
};
struct task_completion_event_auth_flow_result {
    char filler[0xc]; // shared_ptr

    /// @symbol _ZNK4pplx21task_completion_eventIN4xbox8services6system16auth_flow_resultEE3setES4_
    void set(xbox::services::system::auth_flow_result);
};
struct task_completion_event_xbox_live_result_void {
    char filler[0xc]; // shared_ptr

    /// @symbol _ZNK4pplx21task_completion_eventIN4xbox8services16xbox_live_resultIvEEE3setES4_
    void set(xbox::services::xbox_live_result<void>);
};

struct task_impl {
    virtual ~task_impl() = 0;
};
struct task_xbox_live_result_void {
    std::shared_ptr<task_impl> impl;

    /// @symbol _ZNK4pplx4taskIN4xbox8services16xbox_live_resultIvEEE3getEv
    xbox::services::xbox_live_result<void> get();
};
struct task_xbox_live_result_token_and_signature_result {
    std::shared_ptr<task_impl> impl;

    /// @symbol _ZNK4pplx4taskIN4xbox8services16xbox_live_resultINS2_6system26token_and_signature_resultEEEE3getEv
    xbox::services::xbox_live_result<xbox::services::system::token_and_signature_result> get();
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

    /// @symbol _ZN4xbox8services6system11auth_config22set_xtoken_compositionESt6vectorINS1_19token_identity_typeESaIS4_EE
    void set_xtoken_composition(std::vector<xbox::services::system::token_identity_type>);

    mcpe::string const& xbox_live_endpoint();

};

struct auth_manager {

    static std::shared_ptr<xbox::services::system::auth_manager> get_auth_manager_instance();

    void set_rps_ticket(mcpe::string const&);
    pplx::task_xbox_live_result_void initialize_default_nsal();
    /// @symbol _ZN4xbox8services6system12auth_manager32internal_get_token_and_signatureESsRKSsS4_SsRKSt6vectorIhSaIhEEbbS4_
    pplx::task_xbox_live_result_token_and_signature_result internal_get_token_and_signature(mcpe::string, mcpe::string const&, mcpe::string const&, mcpe::string, std::vector<unsigned char> const&, bool, bool, mcpe::string const&);
    std::shared_ptr<xbox::services::system::auth_config> get_auth_config();

};

struct user_auth_android {

    static pplx::task_completion_event_java_rps_ticket* s_rpsTicketCompletionEvent;
    static pplx::task_completion_event_xbox_live_result_void* s_signOutCompleteEvent;

    static std::shared_ptr<xbox::services::system::user_auth_android> get_instance();

    unknown_auth_flow_class* auth_flow;
    int filler;
    mcpe::string xbox_user_id;


};

}
}
}