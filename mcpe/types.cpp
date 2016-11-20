#include <string>
#include <vector>

#include "AppPlatform.h"

AppPlatform** AppPlatform::_singleton = nullptr;
void** AppPlatform::myVtable = nullptr;
void (*AppPlatform::AppPlatform_construct)(AppPlatform*);
void (*AppPlatform::AppPlatform__fireAppFocusGained)(AppPlatform*);
void (*AppPlatform::AppPlatform_initialize)(AppPlatform*);

#include "App.h"

void (*App::App_init)(App*, AppContext&);

#include "MinecraftClient.h"

void (*MinecraftClient::MinecraftClient_construct)(MinecraftClient*, int, char**);
void (*MinecraftClient::MinecraftClient_update)(MinecraftClient*);
void (*MinecraftClient::MinecraftClient_setRenderingSize)(MinecraftClient*, int, int);
void (*MinecraftClient::MinecraftClient_setUISizeAndScale)(MinecraftClient*, int, int, float);

#include "gl.h"

std::string (*gl::getOpenGLVendor)();
std::string (*gl::getOpenGLRenderer)();
std::string (*gl::getOpenGLVersion)();
std::string (*gl::getOpenGLExtensions)();
void (*mce::Platform::OGL::OGL_initBindings)();

#include "Mouse.h"

void (*Mouse::feed)(char, char, short, short, short, short);

#include "Keyboard.h"

void (*Keyboard::Keyboard_feedText)(const std::string&, bool, unsigned char);
std::vector<KeyboardAction>* Keyboard::inputs;
int* Keyboard::states;
