#pragma once

#include <cstdio>
#include <ostream>
#include <vector>

// Something similar to C++23 std::expected
template <typename T>
struct result
{
    bool success;
    T v;
};

// Stream insertion overload for unit tests
template <typename T>
std::ostream &operator<<(std::ostream &o, const std::vector<T> &v)
{
    o << "{ ";
    for (size_t i = 0; i < v.size() - 1; i++)
        o << v[i] << ", ";
    o << v.back();
    o << " }";
    return o;
}

namespace str {

// General

    bool compareSlice(
        const char *start, 
        const char *end, 
        const char *match,  // Null terminated
        bool caseSensitive = true);

    bool beginsWith(
        const char *string, // Null terminated
        const char *match,  // Null terminated
        bool caseSensitive = true);

    const char *quickFormat(const char *fmt, ...);
    // WARNING: The returned value is only valid until the next call, 
    // since it is a pointer to a local buffer (hence the 'quick')

    constexpr unsigned long djb2(const char *start, const char *end)
    {
        unsigned long out = 5381;
        for (const char *c = start; c <= end; c++)
            out = (out << 5) + out + *c;
        return out;
    }

    constexpr unsigned long djb2(const char *string)
    {
        unsigned long out = 5381;
        for (const char *c = string; *c != '\0'; c++)
            out = (out << 5) + out + *c;
        return out;
    }

// Parsing

    enum class ParseError {
        Good,
        Overflow,
        Underflow,
        OutOfRange,
        InvalidConversion,
        IsNullChar,
        IsZero,
        NaN,
        Infinity,
    };

    ParseError toFloat(const char *start, const char *end, float  &out);
    ParseError toSizeT(const char *start, const char *end, size_t &out);
}

