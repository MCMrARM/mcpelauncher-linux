#pragma once

class AppPlatform {

public:
    static void** myVtable;

    static void (*AppPlatform_construct)(AppPlatform*);
    static void (*AppPlatform_initialize)(AppPlatform*);
    static void (*AppPlatform__fireAppFocusGained)(AppPlatform*);

    void** vtable;
    char filler[0x9C - sizeof(void**)]; // 9c
    long long usedMemory, totalMemory, availableMemory; // a4, ac
    char filler2[0x1000];

    AppPlatform() {
        AppPlatform_construct(this);
    }

    static AppPlatform** _singleton;
    void _fireAppFocusGained() {
        AppPlatform__fireAppFocusGained(this);
    }
    void initialize() {
        AppPlatform_initialize(this);
    }

};