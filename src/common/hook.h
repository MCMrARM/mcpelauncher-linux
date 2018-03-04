#pragma once

#include <string>

void addHookLibrary(void* ptr, const std::string& path);
int hookFunction(void* symbol, void* hook, void** original);