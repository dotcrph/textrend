#include "mesh.hpp"

#include "math.hpp"

// Stream insertion overloads for unit tests
std::ostream &operator<<(std::ostream &o, const Polygon &polygon)
{
    o << "{ v indices:";
    for (size_t index : polygon.indices)
        o << ' ' << index;

    o << " }";
    return o;
}

std::ostream &operator<<(std::ostream &o, const Triangle &triangle)
{
    o << "{ v indices:";
    for (int i = 0; i < 3; i++)
        o << ' ' << triangle.indices[i];

    o << " }";
    return o;
}

std::ostream &operator<<(std::ostream &o, const Mesh &mesh)
{
    o << "{\n    vertices (" << mesh.verts.size() << "):\n";
    for (const Vec3f &v : mesh.verts)
        o << "        " << v << '\n';

    o << "\n    faces (" << mesh.faces.size() << "):\n";

    for (const Triangle &t : mesh.faces) {
        o << "        { v:" ;
        for (int i = 0; i < 3; i++)
            o << ' ' << t.indices[i];

        o << " }\n";
    }

    o << "}";
    return o;
}

