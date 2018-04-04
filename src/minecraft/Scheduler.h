#pragma once

#include <chrono>

struct Scheduler {

public:

    static Scheduler* client();

    /// @symbol _ZN9Scheduler17processCoroutinesENSt6chrono8durationIxSt5ratioILx1ELx1000000000EEEE
    void processCoroutines(std::chrono::duration<long long>);


};