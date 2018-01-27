#pragma once

class AppPlatform;
struct AppContext {
    char filler[0x10];
    AppPlatform* platform;
    bool doRender;
};

class App {

public:
    void** vtable;

    void init(AppContext& ctx);

    void quit() {
        ((void (*)(App*)) vtable[25])(this);
    }

    bool wantToQuit() {
        return ((bool (*)(App*)) vtable[26])(this);
    }

};