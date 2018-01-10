#pragma once

#include <chrono>

struct Scheduler {

public:

    static void (*Scheduler_processCoroutines)(Scheduler*, std::chrono::duration<long long>);

    static Scheduler* singleton();

    void processCoroutines(std::chrono::duration<long long> d) {
        Scheduler_processCoroutines(this, d);
    }


};