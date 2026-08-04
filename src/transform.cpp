#include "transform.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

#include "mesh.hpp"
#include "camera.hpp"

namespace transform {
    void worldToClip(
        const Mesh *meshWS,
        Mesh *meshOut,
        const Camera &camera,
        float widthDivHeight)
    {
        assert(meshOut != nullptr 
            && "This function expects an allocated Mesh object");

        assert(meshWS != meshOut);

        assert(meshOut->verts.empty());
        meshOut->verts.reserve(meshWS->verts.size());

        for (Vec3f vert : meshWS->verts) {
            // World to view
            vert += -camera.position;
            vert  =  camera.rotation.conjugate().rotate(vert);

            // View to clip
            vert.x *= camera.getFovHalfCot();
            vert.y *= camera.getFovHalfCot() * widthDivHeight;

            meshOut->verts.push_back(vert);
        }
    }

    void clipToNDC(const Mesh *meshWS, Mesh *meshOut, const Camera &camera)
    {
        assert(meshOut != nullptr 
            && "This function expects an allocated Mesh object");

        assert(meshWS != meshOut);
        assert(meshOut->faces.empty());

        // This stores whether or not a vert at a particular index has been 
        // perspective divided (excluding verts generated after clipping)
        static std::vector<bool> perspectiveDivided;
        perspectiveDivided.resize(meshWS->verts.size());

        std::fill(
            perspectiveDivided.begin(), 
            perspectiveDivided.end(), 
            false
        );

        // This is used to avoid polluting the mesh with intermediate vertices; 
        // the Sutherland-Hodgman algorithm initially puts them here and only 
        // the referenced ones are copied in the mesh after it is done
        static std::vector<Vec3f> newVerts;
        newVerts.clear();

        // Clipping plane surface normal + Z offset
        Vec4f planes[] = {
            { 0,  0,  1,  camera.nearPlane},   // Near
            { 0,  0, -1,  camera.farPlane },   // Far
            { 1,  0,  1,  0},   // Left
            {-1,  0,  1,  0},   // Right
            { 0,  1,  1,  0},   // Bottom
            { 0, -1,  1,  0},   // Top
        };

        for (const Triangle &face : meshWS->faces) {
            size_t indices1[8] = {};
            size_t indices2[8] = {};

            size_t *inIndices  = indices1;
            size_t *outIndices = indices2;

            memcpy(inIndices, face.indices, 3 * sizeof(size_t));

            size_t inSize  = 3;
            size_t outSize = 0;

            // Storing the original amount of verts/normals the mesh had 
            // before adding new vertices/normals for this particular face. 
            // This is used later to recalculate indices for new verts/normals
            size_t indexBoundary = meshOut->verts.size();

            for (const Vec4f &plane : planes) {
                for (size_t i = 0; i < inSize; i++) {
                    size_t j = i > 0 ? i - 1 : inSize - 1;

                    size_t thisIndex = inIndices[i];
                    size_t prevIndex = inIndices[j];

                    Vec3f *thisVert = nullptr;
                    Vec3f *prevVert = nullptr;

                    for (int i = 0; i < 2; i++) {
                        size_t index = i ? thisIndex : prevIndex;
                        Vec3f **var  = i ? &thisVert : &prevVert;
                        Vec3f *array = meshOut->verts.data();

                        if (index >= indexBoundary) {
                            index -= indexBoundary;
                            array = newVerts.data();
                        }

                        *var = &array[index];
                    }

                    Vec3f planeNormal = {plane.x, plane.y, plane.z};
                    Vec3f planePoint  = {0, 0, plane.w};

                    bool thisVertIsVisible 
                        = dot(*thisVert - planePoint, planeNormal) > 0;

                    bool prevVertIsVisible 
                        = dot(*prevVert - planePoint, planeNormal) > 0;

                    bool oneVertIsVisible 
                        = thisVertIsVisible && !prevVertIsVisible 
                       || prevVertIsVisible && !thisVertIsVisible;

                    if (oneVertIsVisible) {
                        Vec3f newVert = Vec3f::zero();

                        bool intersecting = lineSegmentPlaneIntersection(
                            *thisVert,    *prevVert, 
                             planeNormal,  planePoint, 
                             newVert
                        );

                        if (intersecting) {
                            newVerts.push_back(newVert);

                            outIndices[outSize] 
                                = indexBoundary + newVerts.size() - 1;

                            outSize++;
                        }
                    }

                    if (thisVertIsVisible) {
                        outIndices[outSize] = thisIndex;
                        outSize++;
                    } 
                } // for (size_t thisIndex = 0; thisIndex < inSize; ...)

                std::swap(inIndices, outIndices);
                std::swap(inSize,    outSize);

                outSize = 0;
            } // for (const Vec4f &plane : planes)

            assert(inSize != 1 && inSize != 2);

            if (inSize < 3)
                continue;

            // Calculating the surface normal

            // TODO: Currently this thing is set up very messy. Ideally this 
            // should be done inside worldToClip() and then transferred here 
            // somehow. I have no good idea how to do this in a more or less 
            // efficient way (I could pass an std::vector of normals, but this 
            // is horrible for caching and still messy?)
            const Vec3f &vert1 = meshWS->verts[face.indices[0]];
            const Vec3f &vert2 = meshWS->verts[face.indices[1]];
            const Vec3f &vert3 = meshWS->verts[face.indices[2]];

            Vec3f normal = cross(vert3 - vert1, vert2 - vert1);

            // Adding the new verts inside the mesh
            for (size_t i = 0; i < inSize; i++) {
                size_t &index = inIndices[i];

                // Existing vert
                if (index < indexBoundary)
                    continue;

                // New vert
                Vec3f vert = newVerts[index - indexBoundary];

                // Perspective division for new verts
                //
                // I am doing it here because it is the only place where it 
                // is guaranteed without doing extra checks/consuming extra 
                // memory that the vertex has not been perspective divided 
                // before. It is messy, but it is more efficient I think
                vert.x /= vert.z;
                vert.y /= vert.z;

                meshOut->verts.push_back(vert);
                index = meshOut->verts.size() - 1;
            }

            std::vector<Triangle> triangles = earClippingConvex(
                meshOut, 
                inIndices,
                inSize
            );

            for (Triangle &triangle : triangles) {
                triangle.normal = normal;
                meshOut->faces.push_back(triangle);
            }
        } // for (const Triangle &face : meshWS->faces)

        // Perspective division for existing verts
        //
        // I am not doing perspective division for existing verts 
        // in the loop above because it screws up the calculations 
        // for all the triangles except the first one. It took me 
        // almost a month doing nothing except for trying to fix 
        // this particular bug to eventually figure out that this 
        // is what was actually happening. Oof :(
        for (const Triangle &face : meshOut->faces) {
            for (size_t i = 0; i < 3; i++) {
                size_t vIndex = face.indices[i];
                Vec3f &vert   = meshOut->verts[vIndex];

                // Skipping new verts
                if (vIndex >= meshWS->verts.size())
                    continue;

                if (!perspectiveDivided[vIndex]) {
                    vert.x /= vert.z;
                    vert.y /= vert.z;
                    perspectiveDivided[vIndex] = true;
                }
            }
        }
    }

    std::vector<Triangle> earClipping(
        const Mesh *mesh, 
        const Polygon &polygon)
    {
        return earClipping(
            mesh, 
            polygon.indices.data(), 
            polygon.indices.size()
        );
    }

    std::vector<Triangle> earClipping(
        const Mesh *mesh, 
        const size_t *indices,
        const size_t size)
    {
        // NOTE: I am assuming that the order of vertices is 
        // counter-clockwise, because Wavefront OBJ mandates 
        // it and the rest of the program assumes the same order

        assert(size >= 3);

        std::vector<Triangle> out;
        out.reserve(size - 2);

        // This array stores the index of the next vertex for 
        // each of the vertices in indices. I am using this 
        // to avoid deleting items directly, because it is slow
        std::vector<size_t> nextIndex;
        nextIndex.resize(size);

        for (size_t i = 0; i < size - 1; i++)
            nextIndex[i] = i + 1;

        nextIndex.back() = 0;

        // Calculating the surface normal using Newell's method
        Vec3f surfaceNormal = Vec3f::zero();

        for (size_t thisIndex = 0; thisIndex < size; thisIndex++) {
            size_t nextIndex = thisIndex + 1;

            if (nextIndex == size)
                nextIndex = 0;

            size_t thisVertIndex = indices[thisIndex];
            size_t nextVertIndex = indices[nextIndex];

            const Vec3f &thisVert = mesh->verts[thisVertIndex];
            const Vec3f &nextVert = mesh->verts[nextVertIndex];

            surfaceNormal.x += (thisVert.y - nextVert.y) 
                             * (thisVert.z + nextVert.z);

            surfaceNormal.y += (thisVert.z - nextVert.z) 
                             * (thisVert.x + nextVert.x);

            surfaceNormal.z += (thisVert.x - nextVert.x) 
                             * (thisVert.y + nextVert.y);
        }

        size_t currentIndex = 0;

        // NOTE: Limiting the iterations to n^3 to avoid infinite loops
        for (size_t i = 0; i < size * size * size; i++) {
            // Computed index offsets starting from 
            // currentIndex into indices in the polygon
            size_t index0 = currentIndex;
            size_t index1 = nextIndex[index0];
            size_t index2 = nextIndex[index1];

            // Indices
            size_t leftIndex  = indices[index0];
            size_t tipIndex   = indices[index1];
            size_t rightIndex = indices[index2];

            // Handling the last triangle
            if (nextIndex[index2] == index0) {
                out.emplace_back();
                Triangle &newTriangle = out.back();

                newTriangle.indices[0] = leftIndex;
                newTriangle.indices[1] = tipIndex;
                newTriangle.indices[2] = rightIndex;

                return out;
            }

            // Vertices
            const Vec3f &left  = mesh->verts[leftIndex];
            const Vec3f &tip   = mesh->verts[tipIndex];
            const Vec3f &right = mesh->verts[rightIndex];

            // Checking if the angle is < 180
            Vec3f leftVec  = left  - tip;
            Vec3f rightVec = right - tip;

            Vec3f triangleNormal = cross(rightVec, leftVec);

            if (dot(triangleNormal, surfaceNormal) <= 0) {
                currentIndex = nextIndex[currentIndex];
                continue;
            }

            // Checking if some other vert happens to be inside the ear
            bool otherVertIsInsideEar = false;

            for (size_t j = nextIndex[index2]; j != index0; j = nextIndex[j]) {
                size_t index      = indices[j];
                const Vec3f &vert = mesh->verts[index];

                // Checking if the point lies on the triangle plane
                float product = dot(triangleNormal, vert - tip);

                if (!decimalEq(product, 0.0f))
                    continue;

                // Checking if the point lies inside the triangle
                Vec3f edge1Cross = cross(left  - tip,   vert - tip);
                Vec3f edge2Cross = cross(right - left,  vert - left);
                Vec3f edge3Cross = cross(tip   - right, vert - right);

                if (dot(edge1Cross, triangleNormal) < 0
                 && dot(edge2Cross, triangleNormal) < 0
                 && dot(edge3Cross, triangleNormal) < 0
                ) {
                    otherVertIsInsideEar = true;
                    break;
                }
            }

            if (otherVertIsInsideEar) {
                currentIndex = nextIndex[currentIndex];
                continue;
            }

            // Forming a triangle
            out.emplace_back();
            Triangle &newTriangle = out.back();

            newTriangle.indices[0] = leftIndex;
            newTriangle.indices[1] = tipIndex;
            newTriangle.indices[2] = rightIndex;

            // "Removing" the tip from the array
            nextIndex[index0] = index2;
        }

        // TODO: Print a message to the user?
        throw std::logic_error("Triangulation algorithm reached maximum iteration count");
    }

    std::vector<Triangle> earClippingConvex(
        const Mesh *mesh, 
        const Polygon &polygon)
    {
        return earClippingConvex(
            mesh, 
            polygon.indices.data(), 
            polygon.indices.size()
        );
    }

    std::vector<Triangle> earClippingConvex(
        const Mesh *mesh, 
        const size_t *indices,
        const size_t size)
    {
        // NOTE: I am assuming that the order of vertices is 
        // counter-clockwise, because Wavefront OBJ mandates 
        // it and the rest of the program assumes the same order

        assert(size >= 3);

        std::vector<Triangle> out;
        out.reserve(size - 2);

        for (size_t index = 2; index < size; index++) {
            size_t leftIndex  = indices[0];
            size_t tipIndex   = indices[index - 1];
            size_t rightIndex = indices[index];

            out.emplace_back();
            Triangle &newTriangle = out.back();

            newTriangle.indices[0] = leftIndex;
            newTriangle.indices[1] = tipIndex;
            newTriangle.indices[2] = rightIndex;
        }

        return out;
    }
}
