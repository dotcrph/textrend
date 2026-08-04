#pragma once

#include "camera.hpp"
#include "math.hpp"
#include "mesh.hpp"

namespace rasterizer {
    void update();

    void rasterize(const Mesh *meshNDC, const Camera &camera);

    void rasterizeVert(const Vec3f &p, const Mesh *meshNDC);
    void rasterizeEdge(const Vec3f &p1, const Vec3f &p2, const Mesh *meshNDC);

    void rasterizeTriangle(
        const Triangle &face, 
        const Mesh *meshNDC,
        const Camera &camera);

    void fillAlignedTriangle(
        const Vec3f *tip, 
        const Vec3f *right,
        const Vec3f *left, 
        const Vec3f &normal,
        const Camera &camera);

    char gradientFromDot(float d);
}
