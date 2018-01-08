#pragma once

#include <chrono>

struct Scheduler {

public:

    static Scheduler* (*singleton)();
    static void (*Scheduler_processCoroutines)(Scheduler*, std::chrono::duration<long long>);

    void processCoroutines(std::chrono::duration<long long> d) {
        Scheduler_processCoroutines(this, d);
    }


};