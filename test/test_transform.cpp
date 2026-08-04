#include <gtest/gtest.h>
#include "transform.hpp"

#include "utils.hpp" // IWYU pragma: keep
                     // For stream insertion overloads
#include "mesh.hpp"

template <typename T>
constexpr bool cArrayEq(const T *a1, const T *a2, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (a1[i] != a2[i])
            return false;
    }

    return true;
}

//
// transform::earClipping
//

TEST(EarClipping, Sanity)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        {-1,  1,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1, t2;

    t1.indices[0] = 0;
    t1.indices[1] = 1;
    t1.indices[2] = 2;

    t2.indices[0] = 0;
    t2.indices[1] = 2;
    t2.indices[2] = 3;

    m.faces = {t1, t2};

    Polygon p;
    p.indices = {0, 1, 2, 3};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    ASSERT_TRUE(
        cArrayEq(result[0].indices, m.faces[0].indices, 3)
    ) << result;

    ASSERT_TRUE(
        cArrayEq(result[1].indices, m.faces[1].indices, 3)
    ) << result;
}

TEST(EarClipping, Triangle)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1;

    t1.indices[0] = 0;
    t1.indices[1] = 1;
    t1.indices[2] = 2;

    m.faces = {t1};

    Polygon p;
    p.indices = {0, 1, 2};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

TEST(EarClipping, CollinearEdge)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        { 0,  0,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1, t2;

    t1.indices[0] = 1;
    t1.indices[1] = 2;
    t1.indices[2] = 3;

    t2.indices[0] = 1;
    t2.indices[1] = 3;
    t2.indices[2] = 0;

    m.faces = {t1, t2};

    Polygon p;
    p.indices = {0, 1, 2, 3};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

TEST(EarClipping, Concave)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        { 0,  0,  0},
        {-1,  1,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1, t2, t3;

    t1.indices[0] = 1;
    t1.indices[1] = 2;
    t1.indices[2] = 3;

    t2.indices[0] = 1;
    t2.indices[1] = 3;
    t2.indices[2] = 4;

    t3.indices[0] = 1;
    t3.indices[1] = 4;
    t3.indices[2] = 0;

    m.faces = {t1, t2, t3};

    Polygon p;
    p.indices = {0, 1, 2, 3, 4};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

TEST(EarClipping, NonPlanarPolygonEasy)
{
    Mesh m;

    m.verts = {
        { 1,  0,  1},
        { 0,  1,  1},
        {-1,  0,  1},
        {-1,  0, -1},
        { 0,  1, -1},
        { 1,  0, -1},
    };

    Triangle t1, t2, t3, t4;

    t1.indices[0] = 1;
    t1.indices[1] = 2;
    t1.indices[2] = 3;

    t2.indices[0] = 1;
    t2.indices[1] = 3;
    t2.indices[2] = 4;

    t3.indices[0] = 1;
    t3.indices[1] = 4;
    t3.indices[2] = 5;

    t4.indices[0] = 1;
    t4.indices[1] = 5;
    t4.indices[2] = 0;

    m.faces = {t1, t2, t3, t4};

    Polygon p;
    p.indices = {0, 1, 2, 3, 4, 5};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

TEST(EarClipping, NonPlanarPolygonHard)
{
    Mesh m;

    m.verts = {
        { 1,  0,  1},
        { 0,  1,  1},
        {-1,  0,  1},
        {-1,  0, -1},
        { 0, -1, -1},
        { 1,  0, -1},
    };

    Triangle t1, t2, t3, t4;

    t1.indices[0] = 0;
    t1.indices[1] = 1;
    t1.indices[2] = 2;

    t2.indices[0] = 0;
    t2.indices[1] = 2;
    t2.indices[2] = 3;

    t3.indices[0] = 0;
    t3.indices[1] = 3;
    t3.indices[2] = 4;

    t4.indices[0] = 0;
    t4.indices[1] = 4;
    t4.indices[2] = 5;

    m.faces = {t1, t2, t3, t4};

    Polygon p;
    p.indices = {0, 1, 2, 3, 4, 5};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

TEST(EarClipping, PointInsideEar)
{
    Mesh m;

    m.verts = {
        { 1,     1,     0},
        {-1,     1,     0},
        {-1,    -1,     0},
        {-0.5f,  0.5f,  0},
        { 1,    -1,     0},
    };

    Triangle t1, t2, t3;

    t1.indices[0] = 1;
    t1.indices[1] = 2;
    t1.indices[2] = 3;

    t2.indices[0] = 3;
    t2.indices[1] = 4;
    t2.indices[2] = 0;

    t3.indices[0] = 3;
    t3.indices[1] = 0;
    t3.indices[2] = 1;

    m.faces = {t1, t2, t3};

    Polygon p;
    p.indices = {0, 1, 2, 3, 4};

    std::vector<Triangle> result = transform::earClipping(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

//
// transform::earClippingConvex
//

TEST(EarClippingConvex, Sanity)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        {-1,  1,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1, t2;

    t1.indices[0] = 0;
    t1.indices[1] = 1;
    t1.indices[2] = 2;

    t2.indices[0] = 0;
    t2.indices[1] = 2;
    t2.indices[2] = 3;

    m.faces = {t1, t2};

    Polygon p;
    p.indices = {0, 1, 2, 3};

    std::vector<Triangle> result = transform::earClippingConvex(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    ASSERT_TRUE(
        cArrayEq(result[0].indices, m.faces[0].indices, 3)
    ) << result;

    ASSERT_TRUE(
        cArrayEq(result[1].indices, m.faces[1].indices, 3)
    ) << result;
}

TEST(EarClippingConvex, Triangle)
{
    Mesh m;

    m.verts = {
        { 1,  1,  0},
        {-1, -1,  0},
        { 1, -1,  0},
    };

    Triangle t1;

    t1.indices[0] = 0;
    t1.indices[1] = 1;
    t1.indices[2] = 2;

    m.faces = {t1};

    Polygon p;
    p.indices = {0, 1, 2};

    std::vector<Triangle> result = transform::earClippingConvex(&m, p);

    ASSERT_EQ(m.faces.size(), result.size());

    for (size_t i = 0; i < result.size(); i++) {
        ASSERT_TRUE(
            cArrayEq(result[i].indices, m.faces[i].indices, 3)
        ) << result;
    }
}

