#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void* mcpelauncher_hook(void* symbol, void* hook, void** original);
void mcpelauncher_unhook(void* hook);

#ifdef __cplusplus
}
#endif
