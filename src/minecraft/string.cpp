#include "string.h"

mcpe::string* mcpe::string::empty;

mcpe::string::string() {
    ptr = empty->ptr;
}
mcpe::string::string(const std::string &str) {
    if (str.empty()) {
        ptr = empty->ptr;
    } else {
        new (this)std::string(str);
    }
}
mcpe::string::string(const char *str) {
    if (str[0] == '\0') {
        ptr = empty->ptr;
    } else {
        new (this)std::string(str);
    }
}
mcpe::string::string(const string &str) {
    if (str.ptr == empty->ptr) {
        ptr = empty->ptr;
    } else {
        new (this)std::string(*((const std::string *) &str));
    }
}
mcpe::string::~string() {
    if (ptr == empty->ptr)
        return;
    ((std::string*) this)->~basic_string<char>();
}

size_t mcpe::string::length() const {
    if (ptr == empty->ptr)
        return 0;
    return ((std::string*) this)->length();
}

mcpe::string& mcpe::string::operator=(const mcpe::string &str) {
    if (ptr != empty->ptr) {
        if (str.ptr == empty->ptr) {
            ((std::string*) this)->~basic_string<char>();
            new (this)std::string(*((std::string*)&str));
        } else {
            *((std::string*) this) = *((const std::string*) &str);
        }
    } else {
        new (this)std::string(*((std::string*)&str));
    }
}

const char *mcpe::string::c_str() const {
    if (ptr == empty->ptr)
        return "";
    return ((std::string*) this)->c_str();
}