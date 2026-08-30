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
        if (!caching || !isatty(fileno(stream))) {
            fputs(prefix, stream);
            vfprintf(stream, fmt, args);
            putc('\n', stream);
            return;
        }

        // Allocating memory for the string in the cache
        size_t length = strlen(prefix) 
                      + vsnprintf(nullptr, 0, fmt, args) 
                      + 1; // Newline

        if (length > 4096)
            length = 4096;

        size_t bytesLeft = 4096 - bytes;

        char *dest = nullptr;
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

        size_t written = vsnprintf(dest, 4096, fmt, args);

        if (bytes == 4096) {
            memcpy(dest + 4096 - 4, "...\n", 4);
        } else {
            dest[written] = '\n';
        }
    }

    void enableCaching()
    {
        caching = true;
    }

    void flushCache()
    {
        for (char *page : pages) {
            // FIXME: Adding a null terminator at the last moment 
            // is a hack and should be done in a different way
            page[4096] = '\0';

            // NOTE: Since this is printed directly to the terminal 
            // and not piped we can use only a single stream
            puts(page);
        }

        for (char *page : pages) {
            delete[] page;
        }
    }
}
