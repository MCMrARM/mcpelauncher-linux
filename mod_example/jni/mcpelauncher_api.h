#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum LogLevel {
    LOG_TRACE,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};

int mcpelauncher_hook(void* symbol, void* hook, void** original);

void mcpelauncher_log(LogLevel level, const char* tag, const char* fmt, ...);

#ifdef __cplusplus
}
#endif
