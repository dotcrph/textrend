#include "logger.hpp"

#include <cassert>
#include <cstdarg>
#include <cstring>
#include <unistd.h>

#include "args.hpp"

namespace logger {
    // Message cache for when we are using the 
    // alt buffer and want to log something
    bool caching = false;
    std::vector<char *> pages;
    size_t bytes = 0; // Number of bytes occupied in the last page

    void error(const char *fmt, ...)
    {
        if (args::getVerbosity() < 1)
            return;

        va_list args;
        va_start(args, fmt);
        prefixedPrintf(stderr, "[E] ", fmt, args);
        va_end(args);
    }

    void warning(const char *fmt, ...)
    {
        if (args::getVerbosity() < 2)
            return;

        va_list args;
        va_start(args, fmt);
        prefixedPrintf(stderr, "[W] ", fmt, args);
        va_end(args);
    }

    void info(const char *fmt, ...)
    {
        if (args::getVerbosity() < 3)
            return;

        va_list args;
        va_start(args, fmt);
        prefixedPrintf(stderr, "[i] ", fmt, args); // FIXME: Should be stdout
        va_end(args);
    }

    void prefixedPrintf(
        FILE *stream, 
        const char *prefix, 
        const char *fmt, 
        va_list args)
    {
        // Printing the string if we're using the main buffer or piping
        // PERF: isatty calls should be cached
        if (!caching || !isatty(fileno(stream))) {
            fputs(prefix, stream);
            vfprintf(stream, fmt, args);
            putc('\n', stream);
            return;
        }

        // Allocating memory for the string in the cache
        size_t lengthPrefix = strlen(prefix);

        va_list args2;
        va_copy(args2, args);

        size_t length = lengthPrefix + vsnprintf(nullptr, 0, fmt, args2) + 1;

        va_end(args2);

        if (length > 4096)
            length = 4096;

        char *dest       = nullptr;
        size_t bytesLeft = 4096 - bytes;

        if (bytesLeft < length || pages.empty()) {
            // Allocating a new page
            char *page = new char[4096]();

            pages.push_back(page);
            bytes = 0;

            dest = page;
        } else {
            // Using the existing page + offset
            dest = pages.back() + bytes;
        }

        // Replacing the null terminator from 
        // the previous string with a newline
        if (dest - 1 >= pages.back()) {
            assert(dest[-1] == '\0');
            dest[-1] = '\n';
        }

        memcpy(dest, prefix, lengthPrefix);
        vsnprintf(dest + lengthPrefix, 4096, fmt, args);

        if (length == 4096)
            memcpy(dest + 4096 - 4, "...\0", 4);

        bytes += length;
    }

    void enableCaching()
    {
        caching = true;
    }

    void flushCache()
    {
        for (char *page : pages) {
            // NOTE: Since this is printed directly to the 
            // terminal and not piped we can use only stdout
            puts(page);
        }

        for (char *page : pages)
            delete[] page;
    }
}
