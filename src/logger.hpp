#pragma once

#include <cstdio>

namespace logger {
    void error  (const char *fmt, ...);
    void warning(const char *fmt, ...);
    void info   (const char *fmt, ...);

    void prefixedPrintf(
        FILE *stream, 
        const char *prefix, 
        const char *fmt, 
        va_list args);

    void enableCaching();
    void flushCache();
}

