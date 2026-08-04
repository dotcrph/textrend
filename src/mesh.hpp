#pragma once

#include <ostream>
#include <vector>

#include "math.hpp"

///////////////////////////////////////////////////////////////////////////////
// Classes
///////////////////////////////////////////////////////////////////////////////

struct Polygon
{
    std::vector<size_t> indices = {};
};

struct Triangle
{
    size_t indices[3] = {};
    Vec3f normal = Vec3f::zero();
};

struct Mesh
{
    std::vector<Vec3f>    verts{};
    std::vector<Triangle> faces{};

    Vec3f bbMin = Vec3f::zero();
    Vec3f bbMax = Vec3f::zero();

    Mesh() = default;

    Mesh(const Mesh *mesh) : 
        verts(mesh->verts), 
        faces(mesh->faces),
        bbMin(mesh->bbMin),
        bbMax(mesh->bbMax)
    {}
};

// Stream insertion overloads for unit tests
std::ostream &operator<<(std::ostream &o, const Polygon  &polygon);
std::ostream &operator<<(std::ostream &o, const Triangle &triangle);
std::ostream &operator<<(std::ostream &o, const Mesh &mesh);
