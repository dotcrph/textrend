#pragma once

#include "mesh.hpp"
#include "camera.hpp"
#include "utils.hpp"

namespace transform {
    void worldToClip(
        const Mesh *meshWS,
        Mesh *meshOut,
        const Camera &camera,
        float widthDivHeight);

    void clipToNDC(
        const Mesh *meshWS, 
        Mesh *meshOut, 
        const Camera &camera
    );

    result<std::vector<Triangle>> earClipping(
        const Mesh *mesh, 
        const Polygon &polygon);

    result<std::vector<Triangle>> earClipping(
        const Mesh *mesh, 
        const size_t *indices,
        const size_t size);

    std::vector<Triangle> earClippingConvex(
        const Mesh *mesh, 
        const Polygon &polygon);

    std::vector<Triangle> earClippingConvex(
        const Mesh *mesh, 
        const size_t *indices,
        const size_t size);
}
