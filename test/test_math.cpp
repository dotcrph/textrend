#include <gtest/gtest.h>
#include "math.hpp"

#include <cstdlib>

#define assertEqVec(expected, result)                                         \
    ASSERT_TRUE(expected == result)                                           \
        << "Expected: " << expected                                           \
        << "\nResult: " << result

TEST(QuaternionFromEuler, Sanity)
{
    Quaternion result = Quaternion::fromEuler(45, 0, 30);

    Quaternion expected = {
        0.36964383721351624, 
        -0.0990457683801651, 
        0.23911760747432709, 
        0.89239907264709473
    };

    assertEqVec(result, expected);
}

TEST(LineSegmentPlaneIntersection, Sanity)
{
    Vec3f startVert,   endVert;
    Vec3f planeNormal, planePoint;

    bool good;
    Vec3f result, expected;

// 1 - Plane normal parallel to X axis

    startVert = {-2, 0, 0};
    endVert   = { 2, 0, 0};

    planeNormal = {1, 0, 0};
    planePoint  = {1, 0, 0};

    expected = {1, 0, 0};
    result   = Vec3f::zero();

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_TRUE(good);
    assertEqVec(expected, result);

// 2 - Plane normal parallel to X + Y

    startVert = {-1, 0, 0};
    endVert   = { 1, 2, 0};

    planeNormal = {1,    1,    0};
    planePoint  = {0.5f, 0.5f, 0};

    expected = {0, 1, 0};
    result    = Vec3f::zero();

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_TRUE(good);
    assertEqVec(expected, result);

// 3 - Plane normal parallel to X + Y + Z

    startVert = {-2,  1, -2};
    endVert   = { 0,  3,  0};

    planeNormal = {1, 1, 1};
    planePoint  = Vec3f::zero();

    expected = {-1, 2, -1};
    result   = Vec3f::zero();

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_TRUE(good);
    assertEqVec(expected, result);
}

TEST(LineSegmentPlaneIntersection, EdgeCases)
{
    Vec3f startVert,   endVert;
    Vec3f planeNormal, planePoint;

    bool good;
    Vec3f result;

// 1 - The line is parallel to the plane

    startVert = {-1,  1,  0};
    endVert   = {-1, -1,  0};

    planeNormal = {1, 0, 0};
    planePoint  = {0, 0, 0};

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_FALSE(good);

// 2 - The plane contains the line

    startVert = {0,  1,  0};
    endVert   = {0, -1,  0};

    planeNormal = {1, 0, 0};
    planePoint  = {0, 0, 0};

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_FALSE(good);

// 3 - The line is intersecting but the segment is not

    startVert = {-2, 0, 0};
    endVert   = {-1, 0, 0};

    planeNormal = {1, 0, 0};
    planePoint  = {0, 0, 0};

    good = lineSegmentPlaneIntersection(
        startVert,     endVert, 
        planeNormal,   planePoint, 
        result
    );
    
    ASSERT_FALSE(good);
}

