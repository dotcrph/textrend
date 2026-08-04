#pragma once

#include <istream>

#include "mesh.hpp"
#include "utils.hpp"

namespace obj {
    bool parse(std::istream &file, Mesh *mesh);

    bool parseLine(
        const char *c, 
        size_t lineNumber, 
        Mesh *mesh, 
        Vec3f &bbMin, 
        Vec3f &bbMax);

    bool parseVert(
        const char *c, 
        size_t lineNumber, 
        Vec3f &out);

    bool parseFace(
        const char *c, 
        size_t lineNumber, 
        const Mesh *mesh, 
        Polygon &out);

// Utility

    void printVec3fFloatError(
        str::ParseError error, 
        size_t lineNumber,
        int i, 
        const char *c,
        const char *numStart);

    void printFaceIndexError(
        str::ParseError error, 
        size_t lineNumber,
        size_t tupleNumber,
        const char *c,
        const char *numStart);

    str::ParseError parseIndex(
        const char *numStart, 
        const char *numExpectedEnd, 
        size_t arraySize,
        size_t &out);
    // This function expects a 1-based absolute or relative 
    // (negative) index and returns a 0-based absolute index

    void skipWhitespace(const char *&c);
    void skipNonWhitespace(const char *&c);

    bool isWhitespaceOrNull(const char *c);
    // This accepts a pointer to a single char, not a c-string
}
