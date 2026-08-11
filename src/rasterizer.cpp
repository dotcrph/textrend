#include "rasterizer.hpp"

#include <cassert>
#include <cfloat>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <utility>
#include <vector>

#include "camera.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "screen.hpp"
#include "terminal.hpp"

namespace rasterizer {
    bool drawVerts = false;
    bool drawEdges = false;
    bool drawFaces = true;

    bool backfaceCulling = true;

    void update()
    {
        if (terminal::getKeyDown('z'))
            drawVerts = !drawVerts;

        if (terminal::getKeyDown('x'))
            drawEdges = !drawEdges;

        if (terminal::getKeyDown('c'))
            drawFaces = !drawFaces;

        if (terminal::getKeyDown('b'))
            backfaceCulling = !backfaceCulling;
    }

    void rasterize(const Mesh *meshNDC, const Camera &camera)
    {
        for (const Triangle &face : meshNDC->faces) {
            if (backfaceCulling) {
                const Vec3f &vert1 = meshNDC->verts[face.indices[0]];
                const Vec3f &vert2 = meshNDC->verts[face.indices[1]];
                const Vec3f &vert3 = meshNDC->verts[face.indices[2]];

                Vec3f surfaceNormal = cross(vert2 - vert1, vert3 - vert1);

                if (dot(surfaceNormal, Vec3f::forward()) >= 0)
                    continue;
            }

            if (drawFaces)
                rasterizeTriangle(face, meshNDC, camera);

            if (drawEdges) {
                for (int thisIndex = 0; thisIndex < 3; thisIndex++) {
                    size_t prevIndex = thisIndex > 0 
                                     ? thisIndex - 1 
                                     : 2;

                    const Vec3f &p1 = meshNDC->verts[face.indices[prevIndex]];
                    const Vec3f &p2 = meshNDC->verts[face.indices[thisIndex]];

                    rasterizeEdge(p1, p2, meshNDC);
                }
            }
        }

        // PERF: It might be faster to do this in a 
        // loop above because of branch prediction?
        if (drawVerts) {
            for (const Vec3f &vert : meshNDC->verts) {
                if (!inRange(vert.x, -1.0f, 1.0f)
                 || !inRange(vert.y, -1.0f, 1.0f))
                    continue;

                rasterizeVert(vert, meshNDC);
            }
        }
    }

    void rasterizeVert(const Vec3f &p, const Mesh *meshNDC)
    {
        // NOTE: These unary negations are to transform from right-handed 
        // +y up coordinates, which the renderer uses, to left-handed +y 
        // down coordinates, which are used by the terminal. Also, I am 
        // subtracting 0.01f from buffer dimensions because they are 
        // exclusive, i.e. getBufferDimensions().x is an invalid index
        size_t x = (-p.x + 1) / 2 
                 * (screen::getBufferDimensions().x - 0.01f);

        size_t y = (-p.y + 1) / 2 
                 * (screen::getBufferDimensions().y - 0.01f);

        screen::draw(x, y, '@');
    }

    void rasterizeEdge(const Vec3f &p1, const Vec3f &p2, const Mesh *meshNDC)
    {
        int p1x = (-p1.x + 1) / 2 
                * (screen::getBufferDimensions().x - 0.01f);

        int p1y = (-p1.y + 1) / 2 
                * (screen::getBufferDimensions().y - 0.01f);

        int p2x = (-p2.x + 1) / 2 
                * (screen::getBufferDimensions().x - 0.01f);

        int p2y = (-p2.y + 1) / 2 
                * (screen::getBufferDimensions().y - 0.01f);

        int x = p1x;
        int y = p1y;

        // Running Bresenham's algorithm
        int dx = abs(p2x - p1x);
        int dy = abs(p2y - p1y);

        int xStep = p2x > p1x ? 1 : -1;
        int yStep = p2y > p1y ? 1 : -1;

        int *u = &x; // Major axis (with the biggest delta)
        int *v = &y; // Minor axis (with the smallest delta)

        int du = dx; 
        int dv = dy;

        int uStep = xStep;
        int vStep = yStep;

        // Swapping the axes if the slope > 1
        if (du < dv) {
            std::swap(u,     v);
            std::swap(du,    dv);
            std::swap(uStep, vStep);
        }

        int error = (2 * dv) - du;

        for (int i = 0; i < du; i++) {
            screen::draw(x, y, '#');

            if (error > 0) {
                *v += vStep;
                error += 2 * (dv - du);
            } else {
                error += 2 * dv;
            }

            *u += uStep;
        }
    }

    void rasterizeTriangle(
        const Triangle &face, 
        const Mesh *meshNDC,
        const Camera &camera)
    {
        const Vec3f *top    = &meshNDC->verts[face.indices[0]];
        const Vec3f *mid    = &meshNDC->verts[face.indices[1]];
        const Vec3f *bottom = &meshNDC->verts[face.indices[2]];

        // Sorting by Y
        if (top->y < mid->y)
            std::swap(top, mid);

        if (mid->y < bottom->y)
            std::swap(mid, bottom);

        if (top->y < mid->y)
            std::swap(top, mid);

        // Handling the easy cases, i.e. when the triangle 
        // is perfectly flat on the bottom or the top
        bool flat = false;

        const Vec3f *tip   = nullptr;
        const Vec3f *right = nullptr;
        const Vec3f *left  = nullptr;

        if (decimalEq(mid->y, bottom->y)) {
            flat = true;

            tip   = top;
            right = mid;
            left  = bottom;
        }

        if (decimalEq(top->y, mid->y)) {
            flat = true;

            tip   = bottom;
            right = top;
            left  = mid;
        }

        if (flat) {
            if (right->x > left->x)
                std::swap(right, left);

            fillAlignedTriangle(tip, right, left, face.normal, camera);
            return;
        }

        // Handling the general case

        // Splitting the triangle into 2 flat-bottom ones
        float dyTotal = fabsf(bottom->y - top->y);
        float dyMid   = fabsf(mid   ->y - top->y);

        float progress = dyMid / dyTotal;

        float newX = lerp(progress, top->x, bottom->x);

        // FIXME: I kinda don't understand why this should be perspective 
        // corrected. The vert is in NDC, not in screen space, but for 
        // whatever reason this just doesn't give the correct result. 
        // This might indicate some deeper issue or error in the logic
        float newZ = 1 / lerp(progress, 1 / top->z, 1 / bottom->z);

        Vec3f midNew = {newX, mid->y, newZ};

        // Sorting by X
        right = &midNew;
        left  =  mid;

        if (right->x > left->x)
            std::swap(right, left);

        fillAlignedTriangle(top,    right, left, face.normal, camera);
        fillAlignedTriangle(bottom, right, left, face.normal, camera);
    }

    void fillAlignedTriangle(
        const Vec3f *tip, 
        const Vec3f *right,
        const Vec3f *left, 
        const Vec3f &normal,
        const Camera &camera)
    {
        assert(decimalEq(right->y, left->y)
            && "The bottom edge must be perfectly horizontal");

        assert(right->x <= left->x);

        int tipCellX   = (-tip->x + 1) / 2 
                       * (screen::getBufferDimensions().x - 0.01f);

        int tipCellY   = (-tip->y + 1) / 2 
                       * (screen::getBufferDimensions().y - 0.01f);

        int leftCellX  = (-left->x + 1) / 2 
                       * (screen::getBufferDimensions().x - 0.01f);

        int rightCellX = (-right->x + 1) / 2 
                       * (screen::getBufferDimensions().x - 0.01f);

        int baseCellY  = (-left->y + 1) / 2 
                       * (screen::getBufferDimensions().y - 0.01f);

        // Running Bresenham's algorithm for 
        // both of the points at the same time
        int leftX  = leftCellX;
        int rightX = rightCellX;
        int y      = baseCellY;

        int leftDX  = abs(tipCellX - leftCellX );
        int rightDX = abs(tipCellX - rightCellX);

        int baseDX  = leftDX + rightDX;
        int dy      = abs(tipCellY - baseCellY);

        // Ignoring triangles perpendicular to the camera
        if (baseDX == 0 || dy == 0)
            return;

        int leftXStep  = tipCellX > leftCellX  ? 1 : -1;
        int rightXStep = tipCellX > rightCellX ? 1 : -1;
        int yStep      = tipCellY > baseCellY  ? 1 : -1;

        bool leftSteep  = leftDX  < dy;
        bool rightSteep = rightDX < dy;

        int leftError = !leftSteep 
                      ? (2 * dy) - leftDX
                      : (2 * leftDX) - dy;

        int rightError = !rightSteep
                       ? (2 * dy) - rightDX
                       : (2 * rightDX) - dy;

        // Vertical Z interpolation
        float yzStep   = 1.0f / dy;
        float yzFactor = 0;

        // Getting the light value and converting it to a character
        // TODO: Make a proper light source sometime later?
        float lightFactor = dot(normal, {0.0f, -0.6f, 0.8f}) 
                          / normal.magnitude();

        char fill = gradientFromDot(lightFactor);

        for (int i = 0; i <= dy; i++) {
            // Horizontal Z interpolation
            float xzStep   = 1.0f / (rightX - leftX);
            float xzFactor = 0;

            // Drawing a line
            for (int x = leftX; x <= rightX; x++) {
                // Barycentric weights formula
                float z = (1 - xzFactor) * (1 - yzFactor) * (1 / left ->z) 
                        + (    xzFactor) * (1 - yzFactor) * (1 / right->z) 
                        + (    yzFactor) * (1 / tip->z);

                screen::drawDepthTest(x, y, fill, 1 / z);

                xzFactor += xzStep;
            }

            yzFactor += yzStep;

            // Incrementing the major axis and 
            // calculating error for the minor axis
            if (!leftSteep) {
                // Run Bresenham until Y should be changed
                while (leftError < 0) {
                    leftError += 2 * dy;
                    leftX += leftXStep;
                }

                leftError += 2 * (dy - leftDX);
                leftX += leftXStep;
            } else {
                // Run a single step of Bresenham and increment X if needed
                if (leftError > 0) {
                    leftX += leftXStep;
                    leftError += 2 * (leftDX - dy);
                } else {
                    leftError += 2 * leftDX;
                }
            }

            if (!rightSteep) {
                while (rightError < 0) {
                    rightError += 2 * dy;
                    rightX += rightXStep;
                }

                rightError += 2 * (dy - rightDX);
                rightX += rightXStep;
            } else {
                if (rightError > 0) {
                    rightX += rightXStep;
                    rightError += 2 * (rightDX - dy);
                } else {
                    rightError += 2 * rightDX;
                }
            }

            y += yStep;
        }
    }

    char gradientFromDot(float d)
    {
        // Gradient taken from https://paulbourke.net/dataformats/asciiart/
        constexpr char gradient[] = R"($@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvu)" 
                                    R"(nxrjft/|()1{}[]?-_+~<>i!lI;:,"^`'.)";

        // Half-Lambert 
        // NOTE: Subtracting 2 from sizeof(gradient) because it returns 
        // the last index + 1, and the last index is null terminator
        size_t index = (-d + 1) / 2 * (sizeof(gradient) - 2);
        return gradient[index];
    }
}
