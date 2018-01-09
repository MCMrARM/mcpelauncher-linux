#include "log.h"
#include <cstdio>
#include <ctime>

void Log::vlog(LogLevel level, const char* tag, const char* text, va_list args) {
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), text, args);

    char tbuf[128];
    tbuf[0] = '\0';

    time_t t = time(nullptr);
    tm tm;
    localtime_r(&t, &tm);
    strftime(tbuf, sizeof(tbuf), "%H:%M:%S", &tm);
    printf("%s %s [%s] %s\n", tbuf, getLogLevelString(level), tag, buffer);
}