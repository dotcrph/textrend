#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include "utils.hpp"

#include <cstddef>

#include <string>

#define toUnderlying(v) (static_cast<std::underlying_type_t<decltype(v)>>(v))

//
// str::toFloat
//

TEST(ParseFloat, Sanity)
{
    float expected;
    std::string string;

    float result;
    str::ParseError error;

    bool good, eq;

// 1 - General

    expected = 12.345679f;
    string   = "12.345679";

    result = 0;
    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 2 - Zero

    expected = 0.0f;
    string   = "0.0";

    result = 0;
    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 3 - Negative

    expected = -1.23f;
    string   = "-1.23";

    result = 0;
    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 4 - Scientific notation

    expected = 1.1e-4f;
    string   = "1.1e-4";

    result = 0;
    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 5 - Surrounded by text

    expected = 1.0f;
    string   = "Text text 1.0 text text";

    const char *begin = string.c_str() + 10;
    const char *end = begin + 2;

    result = 0;
    error = str::toFloat(begin, end, result);

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 6 - Null char

    string = "";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::IsNullChar, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

TEST(ParseFloat, NamedLiterals)
{
    std::string string;
    str::ParseError error;

    float result;

// 1 - NaN

    string = "-NaN(123)";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::NaN, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 2 - Positive infinity 

    string = "+Inf";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::Infinity, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 3 - Negative infinity 

    string = "-INFINITY";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::Infinity, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

TEST(ParseFloat, UnrepresentableValues)
{
    float result;
    std::string string;
    str::ParseError error;

// 1 - Invalid conversion

    string = "Something something";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + 8, 
        result
    );

    EXPECT_EQ(str::ParseError::InvalidConversion, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 2 - Positive overflow

    // FLT_MAX * 100 + 123
    string = "34028234663852885981170418348451692544123.0";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1,
        result
    );

    EXPECT_EQ(str::ParseError::Overflow, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 3 - Negative overflow

    // Same as above except negative
    string = "-34028234663852885981170418348451692544123.0";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1,
        result
    );

    EXPECT_EQ(str::ParseError::Overflow, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 4 - Underflow

    string = "0.0000000000000000000000000000000000000000000000000000000000001";

    error = str::toFloat(
        string.c_str(), 
        string.c_str() + string.size() - 1,
        result
    );

    EXPECT_EQ(str::ParseError::Underflow, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

//
// str::toSizeT
//

TEST(ParseSizeT, Sanity)
{
    size_t expected;
    std::string string;

    size_t result;
    str::ParseError error;

    bool good, eq;

// 1 - General

    expected = 123456789;
    string   = "123456789";

    result = 0;
    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 2 - Zero

    expected = 0;
    string   = "0";

    result = 1;
    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 3 - Leading zeros

    expected = 123;
    string   = "0000000123";

    result = 0;
    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 5 - Surrounded by text

    expected = 321;
    string   = "Text text 321 text text";

    const char *begin = string.c_str() + 10;
    const char *end = begin + 2;

    result = 0;
    error = str::toSizeT(begin, end, result);

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 6 - Null char

    string = "";

    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::IsNullChar, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

TEST(ParseSizeT, UnrepresentableValues)
{
    size_t result;
    std::string string;
    str::ParseError error;

// 1 - Invalid conversion

    string = "Something something";

    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + 8, 
        result
    );

    EXPECT_EQ(str::ParseError::InvalidConversion, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 2 - Negative value

    string = "-123";

    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        result
    );

    EXPECT_EQ(str::ParseError::InvalidConversion, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 3 - Overflow

    // Max size_t * 100
    string = "1844674407370955161500";

    error = str::toSizeT(
        string.c_str(), 
        string.c_str() + string.size() - 1,
        result
    );

    EXPECT_EQ(str::ParseError::Overflow, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

