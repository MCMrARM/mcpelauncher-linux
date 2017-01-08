#pragma once

class AppPlatform;
struct AppContext {
    char filler[0x10];
    AppPlatform* platform;
    bool doRender;
};

class App {

public:
    static void (*App_init)(App*, AppContext&);

    void** vtable;

    void init(AppContext& ctx) {
        App_init(this, ctx);
    }

    void quit() {
        ((void (*)(App*)) vtable[23])(this);
    }

    bool wantToQuit() {
        return ((bool (*)(App*)) vtable[24])(this);
    }

};