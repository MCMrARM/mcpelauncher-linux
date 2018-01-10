#pragma once

class AppPlatform {

public:
    static void** myVtable;
    static AppPlatform** _singleton;

    void** vtable;
    char filler[0xA0 - sizeof(void**)];
    long long usedMemory, totalMemory, availableMemory;
    char filler2[0x1000];

    AppPlatform();

    void _fireAppFocusGained();

    void initialize();

};