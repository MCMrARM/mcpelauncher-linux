#pragma once

#include <vector>
#include <mutex>

struct OpenSSLMultithreadHelper {

private:
    static std::vector<std::mutex> mutexes;

public:
    static void init();

};