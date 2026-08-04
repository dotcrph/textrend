#pragma once

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
// Declarations                                                              //
///////////////////////////////////////////////////////////////////////////////

// Typedefs and constants
template <typename T> struct Vec2;
template <typename T> struct Vec3;
template <typename T> struct Vec4;
struct Quaternion;

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;

using Vec2s = Vec2<size_t>;
using Vec3s = Vec3<size_t>;
using Vec4s = Vec4<size_t>;

constexpr float pi = 3.14159265359;
constexpr float degToRad = pi / 180;
constexpr float radToDeg = 180 / pi;

constexpr float epsilon  = 0.000001f;

// Structs
template <typename T>
struct Vec2
{
    T x;
    T y;

    Vec2<T> constexpr operator-() const;

    template <typename U>
    Vec2<T> constexpr &operator+=(Vec2<U> lhs);

    template <typename U>
    Vec2<T> constexpr &operator-=(Vec2<U> lhs);

    template <typename U>
    Vec2<T> constexpr operator*(U lhs) const;

    template <typename U>
    Vec2<T> constexpr operator/(U lhs) const;

    explicit constexpr operator Vec3<T>() const;
    explicit constexpr operator Vec4<T>() const;
    explicit constexpr operator Quaternion() const;

    float   constexpr magnitude() const;
    Vec2<T> constexpr normalized() const;

    static constexpr Vec2<T> zero();
    static constexpr Vec2<T> one();

    static constexpr Vec2<T> left();  // +x
    static constexpr Vec2<T> up();    // +y
};

template <typename T>
struct Vec3
{
    T x;
    T y;
    T z;

    Vec3<T> constexpr operator-() const;

    template <typename U>
    Vec3<T> constexpr &operator+=(const Vec3<U> &lhs);

    template <typename U>
    Vec3<T> constexpr &operator-=(const Vec3<U> &lhs);

    template <typename U>
    Vec3<T> constexpr operator*(U lhs) const;

    template <typename U>
    Vec3<T> constexpr operator/(U lhs) const;

    explicit constexpr operator Vec4<T>() const;
    explicit constexpr operator Quaternion() const;

    float   constexpr magnitude() const;
    Vec3<T> constexpr normalized() const;

    static constexpr Vec3<T> zero();
    static constexpr Vec3<T> one();

    static constexpr Vec3<T> left();    // +x
    static constexpr Vec3<T> up();      // +y
    static constexpr Vec3<T> forward(); // +z
};

template <typename T>
struct Vec4
{
    T x;
    T y;
    T z;
    T w;

    Vec4<T> constexpr operator-() const;

    template <typename U>
    Vec4<T> constexpr &operator+=(const Vec4<U> &lhs);

    template <typename U>
    Vec4<T> constexpr &operator-=(const Vec4<U> &lhs);

    template <typename U>
    Vec4<T> constexpr operator*(U lhs) const;

    template <typename U>
    Vec4<T> constexpr operator/(U lhs) const;

    explicit constexpr operator Vec3<T>() const;
    explicit constexpr operator Quaternion() const;

    float   constexpr magnitude() const;
    Vec4<T> constexpr normalized() const;

    static constexpr Vec4<T> zero();
    static constexpr Vec4<T> one();
};

struct Quaternion : public Vec4f
{
    Quaternion constexpr conjugate() const;

    template <typename T>
    constexpr T rotate(const T &point) const;

    template <typename T>
    static constexpr Quaternion fromEuler(const Vec3<T> &eulerDeg);

    template <typename T>
    static constexpr Quaternion fromEuler(T degX, T degY, T degZ);

    static constexpr Quaternion identity();
};

///////////////////////////////////////////////////////////////////////////////
// Vec2                                                                      //
///////////////////////////////////////////////////////////////////////////////

// Member functions

template <typename T>
constexpr Vec2<T> Vec2<T>::operator-() const
{
    return {
        -this->x,
        -this->y,
    };
}

template <typename T>
template <typename U>
constexpr Vec2<T> &Vec2<T>::operator+=(Vec2<U> lhs)
{
    this->x += lhs.x;
    this->y += lhs.y;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec2<T> &Vec2<T>::operator-=(Vec2<U> lhs)
{
    this->x -= lhs.x;
    this->y -= lhs.y;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec2<T> Vec2<T>::operator*(U lhs) const // For numeric types
{
    return {
        this->x * lhs,
        this->y * lhs,
    };
}

template <typename T>
template <typename U>
constexpr Vec2<T> Vec2<T>::operator/(U lhs) const // For numeric types
{
    return {
        this->x / lhs,
        this->y / lhs,
    };
}

template <typename T>
constexpr Vec2<T>::operator Vec3<T>() const
{
    return {
        .x = this->x,
        .y = this->y,
        .z = 0,
    };
}

template <typename T>
constexpr Vec2<T>::operator Vec4<T>() const
{
    return {
        .x = this->x,
        .y = this->y,
        .z = 0,
        .w = 0,
    };
}

template <typename T>
constexpr Vec2<T>::operator Quaternion() const
{
    return {
        static_cast<float>(this->x),
        static_cast<float>(this->y),
        0,
        0,
    };
}

template <typename T>
constexpr float Vec2<T>::magnitude() const
{
    auto x2 = this->x * this->x;
    auto y2 = this->y * this->y;

    return sqrt(x2 + y2);
}

template <typename T>
constexpr Vec2<T> Vec2<T>::normalized() const
{
    float magnitude = this->magnitude();

    return {
        this->x / magnitude,
        this->y / magnitude,
    };
}

template <typename T>
constexpr Vec2<T> Vec2<T>::zero()
{
    return {0, 0};
}

template <typename T>
constexpr Vec2<T> Vec2<T>::one()
{
    return {1, 1};
}

template <typename T>
constexpr Vec2<T> Vec2<T>::left()
{
    return {1, 0};
}

template <typename T>
constexpr Vec2<T> Vec2<T>::up()
{
    return {0, 1};
}

// Free functions

template <typename T>
constexpr Vec2<T> operator+(Vec2<T> lhs, Vec2<T> rhs)
{
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
    };
}

template <typename T>
constexpr Vec2<T> operator-(Vec2<T> lhs, Vec2<T> rhs)
{
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
    };
}

template <typename T>
constexpr bool operator==(Vec2<T> lhs, Vec2<T> rhs)
{
    return lhs.x == rhs.x 
           && lhs.y == rhs.y;
}

template <typename T>
constexpr bool operator!=(Vec2<T> lhs, Vec2<T> rhs)
{
    return !(lhs == rhs);
}

template <typename T>
std::ostream &operator<<(std::ostream &o, Vec2<T> v)
{
    o << "{ " 
      << v.x << ", " 
      << v.y 
      << " }";

    return o;
}

template <typename T>
constexpr float dot(Vec2<T> lhs, Vec2<T> rhs)
{
    return lhs.x * rhs.x 
         + lhs.y * rhs.y;
}

template <typename T>
constexpr float cross(Vec2<T> lhs, Vec2<T> rhs)
{
    return lhs.x * rhs.y 
         - lhs.y * rhs.x;
}

///////////////////////////////////////////////////////////////////////////////
// Vec3                                                                      //
///////////////////////////////////////////////////////////////////////////////

// Member functions

template <typename T>
constexpr Vec3<T> Vec3<T>::operator-() const
{
    return {
        -this->x,
        -this->y,
        -this->z,
    };
}

template <typename T>
template <typename U>
constexpr Vec3<T> &Vec3<T>::operator+=(const Vec3<U> &lhs)
{
    this->x += lhs.x;
    this->y += lhs.y;
    this->z += lhs.z;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec3<T> &Vec3<T>::operator-=(const Vec3<U> &lhs)
{
    this->x -= lhs.x;
    this->y -= lhs.y;
    this->z -= lhs.z;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec3<T> Vec3<T>::operator*(U lhs) const // For numeric types
{
    return {
        this->x * lhs,
        this->y * lhs,
        this->z * lhs,
    };
}

template <typename T>
template <typename U>
constexpr Vec3<T> Vec3<T>::operator/(U lhs) const // For numeric types
{
    return {
        this->x / lhs,
        this->y / lhs,
        this->z / lhs,
    };
}


template <typename T>
constexpr Vec3<T>::operator Vec4<T>() const
{
    return {
        .x = this->x,
        .y = this->y,
        .z = this->z,
        .w = 0,
    };
}

template <typename T>
constexpr Vec3<T>::operator Quaternion() const
{
    return {
        static_cast<float>(this->x),
        static_cast<float>(this->y),
        static_cast<float>(this->z),
        0,
    };
}

template <typename T>
constexpr float Vec3<T>::magnitude() const
{
    auto x2 = this->x * this->x;
    auto y2 = this->y * this->y;
    auto z2 = this->z * this->z;

    return sqrt(x2 + y2 + z2);
}

template <typename T>
constexpr Vec3<T> Vec3<T>::normalized() const
{
    float magnitude = this->magnitude();

    return {
        this->x / magnitude,
        this->y / magnitude,
        this->z / magnitude,
    };
}

template <typename T>
constexpr Vec3<T> Vec3<T>::zero()
{
    return {0, 0, 0};
}

template <typename T>
constexpr Vec3<T> Vec3<T>::one()
{
    return {1, 1, 1};
}

template <typename T>
constexpr Vec3<T> Vec3<T>::left()
{
    return {1, 0, 0};
}

template <typename T>
constexpr Vec3<T> Vec3<T>::up()
{
    return {0, 1, 0};
}

template <typename T>
constexpr Vec3<T> Vec3<T>::forward()
{
    return {0, 0, 1};
}

// Free functions

template <typename T>
constexpr Vec3<T> operator+(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
    };
}

template <typename T>
constexpr Vec3<T> operator-(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
    };
}

template <typename T>
constexpr bool operator==(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return lhs.x == rhs.x 
           && lhs.y == rhs.y
           && lhs.z == rhs.z;
}

template <typename T>
constexpr bool operator!=(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return !(lhs == rhs);
}

template <typename T>
std::ostream &operator<<(std::ostream &o, const Vec3<T> &v)
{
    o << "{ " 
      << v.x << ", " 
      << v.y << ", " 
      << v.z 
      << " }";

    return o;
}

template <typename T>
constexpr float dot(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return lhs.x * rhs.x 
         + lhs.y * rhs.y 
         + lhs.z * rhs.z;
}

template <typename T>
constexpr Vec3<T> cross(const Vec3<T> &lhs, const Vec3<T> &rhs)
{
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

///////////////////////////////////////////////////////////////////////////////
// Vec4                                                                      //
///////////////////////////////////////////////////////////////////////////////

// Member functions

template <typename T>
constexpr Vec4<T> Vec4<T>::operator-() const
{
    return {
        -this->x,
        -this->y,
        -this->z,
        -this->w,
    };
}

template <typename T>
template <typename U>
constexpr Vec4<T> &Vec4<T>::operator+=(const Vec4<U> &lhs)
{
    this->x += lhs.x;
    this->y += lhs.y;
    this->z += lhs.z;
    this->w += lhs.w;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec4<T> &Vec4<T>::operator-=(const Vec4<U> &lhs)
{
    this->x -= lhs.x;
    this->y -= lhs.y;
    this->z -= lhs.z;
    this->w -= lhs.w;
    return *this;
}

template <typename T>
template <typename U>
constexpr Vec4<T> Vec4<T>::operator*(U lhs) const // For numeric types
{
    return {
        this->x * lhs,
        this->y * lhs,
        this->z * lhs,
        this->w * lhs,
    };
}

template <typename T>
template <typename U>
constexpr Vec4<T> Vec4<T>::operator/(U lhs) const // For numeric types
{
    return {
        this->x / lhs,
        this->y / lhs,
        this->z / lhs,
        this->w / lhs,
    };
}

template <typename T>
constexpr Vec4<T>::operator Vec3<T>() const
{
    return {
        .x = this->x,
        .y = this->y,
        .z = this->z,
    };
}

template <typename T>
constexpr Vec4<T>::operator Quaternion() const
{
    return {
        static_cast<float>(this->x),
        static_cast<float>(this->y),
        static_cast<float>(this->z),
        static_cast<float>(this->w),
    };
}

template <typename T>
constexpr float Vec4<T>::magnitude() const
{
    auto x2 = this->x * this->x;
    auto y2 = this->y * this->y;
    auto z2 = this->z * this->z;
    auto w2 = this->w * this->w;

    return sqrt(x2 + y2 + z2 + w2);
}

template <typename T>
constexpr Vec4<T> Vec4<T>::normalized() const
{
    float magnitude = this->magnitude();

    return {
        this->x / magnitude,
        this->y / magnitude,
        this->z / magnitude,
        this->w / magnitude,
    };
}

template <typename T>
constexpr Vec4<T> Vec4<T>::zero()
{
    return {0, 0, 0, 0};
}

template <typename T>
constexpr Vec4<T> Vec4<T>::one()
{
    return {1, 1, 1, 1};
}

// Free functions

template <typename T>
constexpr Vec4<T> operator+(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
    return {
        lhs.x + rhs.x,
        lhs.y + rhs.y,
        lhs.z + rhs.z,
        lhs.w + rhs.w,
    };
}

template <typename T>
constexpr Vec4<T> operator-(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
    return {
        lhs.x - rhs.x,
        lhs.y - rhs.y,
        lhs.z - rhs.z,
        lhs.w - rhs.w,
    };
}

template <typename T>
constexpr bool operator==(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
    return lhs.x == rhs.x 
           && lhs.y == rhs.y
           && lhs.z == rhs.z
           && lhs.w == rhs.w;
}

template <typename T>
constexpr bool operator!=(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
    return !(lhs == rhs);
}

template <typename T>
std::ostream &operator<<(std::ostream &o, const Vec4<T> &v)
{
    o << "{ " 
      << v.x << ", " 
      << v.y << ", " 
      << v.z << ", " 
      << v.w 
      << " }";

    return o;
}

template <typename T>
constexpr float dot(const Vec4<T> &lhs, const Vec4<T> &rhs)
{
    return lhs.x * rhs.x 
         + lhs.y * rhs.y 
         + lhs.z * rhs.z
         + lhs.w * rhs.w;
}

///////////////////////////////////////////////////////////////////////////////
// Quaternion                                                                //
///////////////////////////////////////////////////////////////////////////////

// Free functions

constexpr Quaternion hamilton(const Quaternion &lhs, const Quaternion &rhs)
{
    return {
        lhs.w * rhs.x + rhs.w * lhs.x + lhs.y * rhs.z - rhs.y * lhs.z,
        lhs.w * rhs.y + rhs.w * lhs.y + lhs.z * rhs.x - rhs.z * lhs.x,
        lhs.w * rhs.z + rhs.w * lhs.z + lhs.x * rhs.y - rhs.x * lhs.y,
        lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
    };
}

// Member functions

constexpr Quaternion Quaternion::conjugate() const
{
    return {
        -this->x,
        -this->y,
        -this->z,
        this->w,
    };
}

template <typename T>
constexpr T Quaternion::rotate(const T &point) const
{
    Quaternion q = *this;
    Quaternion p = static_cast<Quaternion>(point);

    Quaternion first  = hamilton(q, p);
    Quaternion second = hamilton(first, q.conjugate());

    return static_cast<T>(second);
}

template <typename T>
constexpr Quaternion Quaternion::fromEuler(const Vec3<T> &eulerDeg)
{
    Vec3f eulerRad = eulerDeg * degToRad;

    float sinX = sin(eulerRad.x / 2);
    float cosX = cos(eulerRad.x / 2);

    float sinY = sin(eulerRad.y / 2);
    float cosY = cos(eulerRad.y / 2);

    float sinZ = sin(eulerRad.z / 2);
    float cosZ = cos(eulerRad.z / 2);

    return {
        sinX * cosY * cosZ - cosX * sinY * sinZ,
        cosX * sinY * cosZ - sinX * cosY * sinZ,
        cosX * cosY * sinZ - sinX * sinY * cosZ,
        cosX * cosY * cosZ - sinX * sinY * sinZ,
    };
}

template <typename T>
constexpr Quaternion Quaternion::fromEuler(T degX, T degY, T degZ)
{
    float radX = degX * degToRad;
    float radY = degY * degToRad;
    float radZ = degZ * degToRad;

    float sinX = sin(radX / 2);
    float cosX = cos(radX / 2);

    float sinY = sin(radY / 2);
    float cosY = cos(radY / 2);

    float sinZ = sin(radZ / 2);
    float cosZ = cos(radZ / 2);

    return {
        sinX * cosY * cosZ - cosX * sinY * sinZ,
        cosX * sinY * cosZ - sinX * cosY * sinZ,
        cosX * cosY * sinZ - sinX * sinY * cosZ,
        cosX * cosY * cosZ - sinX * sinY * sinZ,
    };
}

constexpr Quaternion Quaternion::identity()
{
    return {0, 0, 0, 1};
}

///////////////////////////////////////////////////////////////////////////////
// Utility functions                                                         //
///////////////////////////////////////////////////////////////////////////////

template <typename T>
constexpr bool decimalEq(T lhs, T rhs, T error = epsilon)
{
    return std::abs(lhs - rhs) < error;
}

template <typename T>
constexpr T min(T lhs, T rhs)
{
    if (lhs < rhs)
        return lhs;

    return rhs;
}

template <typename T>
constexpr T max(T lhs, T rhs)
{
    if (lhs > rhs)
        return lhs;

    return rhs;
}

template <typename T>
constexpr bool inRange(T val, T min, T max, bool inclusive = true)
{
    if (min > max)
        std::swap(min, max);

    if (inclusive) {
        return val >= min && val <= max;
    } else {
        assert(min <= max - 2);
        return val > min && val < max;
    }
}

template <typename T>
constexpr T clamp(T val, T min, T max)
{
    if (min > max)
        std::swap(min, max);

    if (val < min)
        return min;

    if (val > max)
        return max;

    return val;
}

template <typename T>
constexpr T lerp(float t, T a, T b)
{
    clamp(t, 0.0f, 1.0f);
    return a + (b - a) * t;
}

template <typename T>
constexpr T remap(T val, T inLow, T inHigh, T outLow, T outHigh)
{
    return outLow + (val - inLow) * (outHigh - outLow) / (inHigh - inLow);
}

///////////////////////////////////////////////////////////////////////////////
// Geometry
///////////////////////////////////////////////////////////////////////////////

constexpr bool lineSegmentPlaneIntersection(
    const Vec3f &startVert, 
    const Vec3f &endVert, 
    const Vec3f &planeNormal, 
    const Vec3f &planePoint, 
    Vec3f &outVert)
{
    float denominator = dot(planeNormal, endVert - startVert);

    // The line is parallel to the plane
    if (decimalEq(denominator, 0.0f))
        return false;

    float t = dot(planeNormal, planePoint - startVert) / denominator;

    // The line is intersecting but the segment is not
    if (t < 0.0f - epsilon || t > 1.0f + epsilon)
        return false;

    outVert = startVert + (endVert - startVert) * t;
    return true;
}

