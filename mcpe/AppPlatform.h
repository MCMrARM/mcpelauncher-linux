#pragma once

class AppPlatform {

public:
    static void** myVtable;

    static void (*AppPlatform_construct)(AppPlatform*);
    static void (*AppPlatform__fireAppFocusGained)(AppPlatform*);

    void** vtable;
    char filler [0x50+100];

    AppPlatform() {
        AppPlatform_construct(this);
    }

    static AppPlatform** _singleton;
    void _fireAppFocusGained() {
        AppPlatform__fireAppFocusGained(this);
    }

};