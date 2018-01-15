#pragma once

class AppPlatform {

public:
    /// @symbol _ZTV11AppPlatform
    static void** myVtable;

    void** vtable;
    char filler[0xA0 - sizeof(void**)];
    long long usedMemory, totalMemory, availableMemory;
    char filler2[0x1000];

    AppPlatform();

    void _fireAppFocusGained();

    void initialize();

};