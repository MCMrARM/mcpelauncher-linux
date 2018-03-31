#pragma once

#include <string>
#include <cstring>
#include <atomic>

namespace mcpe {

struct string {

private:
    struct _Rep {
        size_t length;
        size_t capacity;
        std::atomic<int> refcount;

        void addRef() {
            refcount++;
        }
        void removeRef() {
            if (refcount.fetch_sub(1) == 0)
                delete this;
        }
    };

    void* ptr;

    static _Rep* createRep(const char* data, size_t length, size_t capacity = 0);

    _Rep* getRep() const {
        return (_Rep*) ptr - 1;
    }

    void initRep(_Rep* rep) {
        rep->addRef();
        ptr = (void*) (rep + 1);
    }

    void assignRep(_Rep* rep) {
        if (rep != empty->getRep())
            rep->addRef();
        if (ptr != empty->ptr)
            getRep()->removeRef();
        ptr = (void*) (rep + 1);
    }

public:
    static mcpe::string* empty;

    string();
    string(const char *str, size_t len);
    string(const string &str);
    inline string(const char *str) : string(str, strlen(str)) { }
    inline string(const std::string &str) : string(str.c_str(), str.length()) {}
    ~string();

    string &operator=(const string &str);

    size_t length() const;
    const char *c_str() const;

    //string operator+(const string &str);

    bool operator==(const string &s) const {
        if (s.ptr == ptr)
            return true;
        if (s.length() != length())
            return false;
        return (memcmp(c_str(), s.c_str(), length()) == 0);
    }

    bool operator<(const string& s) const {
        int d = memcmp(c_str(), s.c_str(), std::min(length(), s.length()));
        if (d < 0) return true;
        if (d == 0) return length() < s.length();
        return false;
    }

    inline std::string std() const {
        return std::string(c_str(), length());
    }

};

}

std::ostream& operator<<(std::ostream&, const mcpe::string&);
