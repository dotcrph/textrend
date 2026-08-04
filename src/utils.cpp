#include "utils.hpp"
#include "args.hpp"

#include <cassert>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>

namespace str {

// General

    bool compareSlice(
        const char *start, 
        const char *end, 
        const char *match,
        bool caseSensitive)
    {
        if (caseSensitive)
            return strncmp(start, match, end - start + 1) == 0;

        for (size_t i = 0; i <= end - start; i++) {
            if (match[i] == '\0')
                return false;

            assert(start[i] != '\0');

            char c1 = tolower(start[i]);
            char c2 = tolower(match[i]);

            if (c1 != c2)
                return false;
        }

        return true;
    }

    bool beginsWith(
        const char *string, 
        const char *match,
        bool caseSensitive)
    {
        size_t len = strlen(match);

        if (caseSensitive)
            return strncmp(string, match, len) == 0;

        for (size_t i = 0; i < len; i++) {
            if (string[i] == '\0')
                return false;

            char c1 = tolower(string[i]);
            char c2 = tolower(match[i]);

            if (c1 != c2)
                return false;
        }

        return true;
    }

    const char *quickFormat(const char *fmt, ...)
    {
        static std::vector<char> buffer;

        va_list args;
        va_start(args, fmt);

        va_list args1;
        va_copy(args1, args);
        int size = vsnprintf(nullptr, 0, fmt, args1);
        va_end(args1);

        buffer.resize(size + 1);

        va_list args2;
        va_copy(args2, args);
        vsnprintf(buffer.data(), buffer.size(), fmt, args);
        va_end(args2);

        va_end(args);
        return buffer.data();
    }

// Parsing

    ParseError toFloat(const char *start, const char *end, float &out)
    {
        if (*start == '\0')
            return ParseError::IsNullChar;

        // Discarding infinity and NaN

        // NOTE: I'm matching NAN and INFINITY as strings instead of checking 
        // the result of strtof() because I'm compiling with -ffast-math 

        const char *startNoSign = start;
        if (*start == '-' || *start == '+')
            startNoSign++;

        // The reason why I'm checking the prefix is the fact 
        // that strtof accepts "NAN or NANsequence [...], where 
        // each character is either an alphanumeric character 
        // (as in isalnum) or the underscore character (_)."
        bool isNan = str::beginsWith(startNoSign, "nan", false);

        if (isNan)
            return ParseError::NaN;

        bool isInf = compareSlice(startNoSign, end, "inf", false);

        if (isInf)
            return ParseError::Infinity;

        bool isInfinity = compareSlice(startNoSign, end, "infinity", false);

        if (isInfinity)
            return ParseError::Infinity;

        // Converting the value to float now
        char *actualEnd = nullptr; // Set by strtof()

        errno = 0;
        float result = strtof(start, &actualEnd);

        actualEnd--; // Strtof() gives a pointer one 
                     // past the end of the lexeme

        if (errno == ERANGE) {
            if (result == HUGE_VALF || result == -HUGE_VALF)
                return ParseError::Overflow;

            // FIXME: This is unreliable, because some platforms might not 
            // set errno to ERANGE when underflow happens. As of now, I have 
            // only tested this code on glibc 2.39-0ubuntu8.6, and it seems 
            // to be working. I do not really have a better way of detecting 
            // it, especially since I am compiling with -ffast-math. The worst 
            // case scenario for the user is that they will recieve a 0.0 
            // instead of an error, which is bad but (I hope) not catastrophic
            return ParseError::Underflow;
        }

        if (end != actualEnd)
            return ParseError::InvalidConversion;

        out = result;
        return ParseError::Good;
    }

    ParseError toSizeT(const char *start, const char *end, size_t &out)
    {
        if (*start == '\0')
            return ParseError::IsNullChar;

        size_t result = 0;

        for (const char *c = start; c <= end; c++) {
            if (!isdigit(*c))
                return ParseError::InvalidConversion;

            size_t digit = *c - '0';

            if ((SIZE_MAX - digit) / 10 < result)
                return ParseError::Overflow;

            result *= 10;
            result += digit;
        }

        out = result;
        return ParseError::Good;
    }
}

namespace logger {
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
        prefixedPrintf(stderr, "[i] ", fmt, args);
        va_end(args);
    }

    void prefixedPrintf(
        FILE *stream, 
        const char *prefix, 
        const char *fmt, 
        va_list args)
    {
        fputs(prefix, stream);
        vfprintf(stream, fmt, args);
        putc('\n', stream);
    }
}
