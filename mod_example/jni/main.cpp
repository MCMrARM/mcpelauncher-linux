#include <iostream>

#include "mcpelauncher_api.h"

struct Common {
    static std::string getGameVersionString();
    static std::string (*$getGameVersionString)();
    static std::string $$getGameVersionString() {
        return "Hello world!";
    }
};
std::string (*Common::$getGameVersionString)();

extern "C" {

void mod_init() {
    std::cout << "init test mod\n";
    mcpelauncher_hook((void*) &Common::getGameVersionString, (void*) &Common::$$getGameVersionString, (void**) &Common::$getGameVersionString);
}

}