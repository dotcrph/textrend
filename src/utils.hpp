#pragma once

#include <cstdio>
#include <ostream>
#include <vector>

#define concatImpl(a, b) a##b
#define concat(a, b) concatImpl(a, b)

class DeferClass
{
private:
    void (*f)();
public:
    DeferClass(void (*f)()) : f(f) {}
    ~DeferClass() { f(); }
};

#define defer(funcPtr) \
    DeferClass concat(_defer_, __COUNTER__)(funcPtr)

// Using a template + a factory function because it does 
// not allocate things on the heap unlike std::function 
// (thanks http://the-witness.net/news/2012/11/scopeexit-in-c11/)
template <typename F>
class DeferTemplatedClass
{
private:
    F f;
public:
    DeferTemplatedClass(F f) : f(f) {}
    ~DeferTemplatedClass() { f(); }
};

template <typename F>
DeferTemplatedClass<F> CreateDeferTemplated(F f)
{
    return DeferTemplatedClass<F>(f);
}

// This kinda breaks the camelCase convention, but I like it more 
// because this follows the conventions of the built-in keywords
#define defer_lambda(body) \
    auto concat(_defer_lambda_, __COUNTER__) \
        = CreateDeferTemplated([&](){body;})

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

namespace logger {
    void error  (const char *fmt, ...);
    void warning(const char *fmt, ...);
    void info   (const char *fmt, ...);

    void prefixedPrintf(
        FILE *stream, 
        const char *prefix, 
        const char *fmt, 
        va_list args);
}

