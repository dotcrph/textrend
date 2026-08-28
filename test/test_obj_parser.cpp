#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include "obj_parser.hpp"

#include <cstddef>

#include <sstream>
#include <string>

#include "mesh.hpp"
#include "utils.hpp"

#define toUnderlying(v) (static_cast<std::underlying_type_t<decltype(v)>>(v))

///////////////////////////////////////////////////////////////////////////////
// Parsing functions
///////////////////////////////////////////////////////////////////////////////

TEST(ObjParser, GeneralCube)
{
    testing::internal::CaptureStderr();

    std::stringstream file(
R"(# Blender v2.76 (sub 0) OBJ File: ''
# www.blender.org
mtllib cube.mtl
o Cube
v 1.000000 -1.000000 -1.000000
v 1.000000 -1.000000 1.000000
v -1.000000 -1.000000 1.000000
v -1.000000 -1.000000 -1.000000
v 1.000000 1.000000 -0.999999
v 0.999999 1.000000 1.000001
v -1.000000 1.000000 1.000000
v -1.000000 1.000000 -1.000000
vt 1.000000 0.333333
vt 1.000000 0.666667
vt 0.666667 0.666667
vt 0.666667 0.333333
vt 0.666667 0.000000
vt 0.000000 0.333333
vt 0.000000 0.000000
vt 0.333333 0.000000
vt 0.333333 1.000000
vt 0.000000 1.000000
vt 0.000000 0.666667
vt 0.333333 0.333333
vt 0.333333 0.666667
vt 1.000000 0.000000
vn 0.000000 -1.000000 0.000000
vn 0.000000 1.000000 0.000000
vn 1.000000 0.000000 0.000000
vn -0.000000 0.000000 1.000000
vn -1.000000 -0.000000 -0.000000
vn 0.000000 0.000000 -1.000000
usemtl Material
s off
f 2/1/1 3/2/1 4/3/1
f 8/1/2 7/4/2 6/5/2
f 5/6/3 6/7/3 2/8/3
f 6/8/4 7/5/4 3/4/4
f 3/9/5 7/10/5 8/11/5
f 1/12/6 4/13/6 8/11/6
f 1/4/1 2/1/1 4/3/1
f 5/14/2 8/1/2 6/5/2
f 1/12/3 5/6/3 2/8/3
f 2/12/4 6/8/4 3/4/4
f 4/13/5 3/9/5 8/11/5
f 5/6/6 1/12/6 8/11/6)"
    );

    Mesh *result = new Mesh();
    defer(delete result);

    obj::parse(file, result);

    ASSERT_NE(result, nullptr) << testing::internal::GetCapturedStderr();
}

///////////////////////////////////////////////////////////////////////////////
// Utility functions
///////////////////////////////////////////////////////////////////////////////

//
// obj::parseIndex
//

TEST(ParseIndex, Sanity)
{
    size_t arraySize;
    size_t expected;
    std::string string;

    size_t result;
    str::ParseError error;

    bool good, eq;

// 1 - General

    arraySize = 50;
    expected  = 14;   // See declaration of obj::parse index for 
    string    = "15"; // explanation why we expect a given number - 1

    result = 0;
    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 2 - First index 

    arraySize = 10;
    expected  = 0;
    string    = "1";

    result = 0;
    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 3 - Relative

    arraySize = 10;
    expected  = arraySize - 1;
    string    = "-1";

    result = 0;
    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    good = error == str::ParseError::Good;
    eq   = expected == result;

    EXPECT_TRUE(good && eq) 
        << "\nError: " << toUnderlying(error)
        << "\nExpected: " << expected
        << "\nResult: " << result;

// 4 - Null char

    arraySize = 5;
    string = "";

    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    EXPECT_EQ(str::ParseError::IsNullChar, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

TEST(ParseIndex, UnrepresentableValues)
{
    size_t arraySize;
    size_t result;
    std::string string;
    str::ParseError error;

// 1 - Empty array

    arraySize = 0;
    string    = "2";

    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    EXPECT_EQ(str::ParseError::OutOfRange, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 2 - Zero

    arraySize = 10;
    string = "0";

    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    EXPECT_EQ(str::ParseError::IsZero, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 3 - Out of range

    arraySize = 5;
    string    = "25";

    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    EXPECT_EQ(str::ParseError::OutOfRange, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;

// 4 - Relative out of range

    arraySize = 5;
    string    = "-25";

    error = obj::parseIndex(
        string.c_str(), 
        string.c_str() + string.size() - 1, 
        arraySize,
        result
    );

    EXPECT_EQ(str::ParseError::OutOfRange, error)
        << "\nError: " << toUnderlying(error)
        << "\nResult: " << result;
}

