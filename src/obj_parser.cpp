#include "obj_parser.hpp"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <string>

#include "math.hpp"
#include "mesh.hpp"
#include "utils.hpp"
#include "transform.hpp"

namespace obj {
    bool parse(std::istream &file, Mesh *mesh)
    {
        assert(file.good());
        assert(mesh != nullptr);

        std::string line;
        size_t lineNumber = 1;

        int errorCount = 0;
        constexpr int errorLimit = 50;

        Vec3f bbMin = { FLT_MAX,  FLT_MAX,  FLT_MAX};
        Vec3f bbMax = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        while (std::getline(file, line)) {
            // Handling CRLF manually because the user could easily 
            // open a file saved on Windows on a Mac, for example
            if (!line.empty() && line.back() == '\r')
                line.back() = '\0';

            bool good = parseLine(
                line.c_str(), 
                lineNumber, 
                mesh, 
                bbMin, 
                bbMax
            );

            lineNumber++;

            if (!good)
                errorCount++;

            if (errorCount >= errorLimit) {
                logger::error(
                    "OBJ Parser @ ln %zu: Too many errors, stopping",
                    lineNumber
                );

                return false;
            }
        }

        if (errorCount != 0)
            return false;

        mesh->bbMin = bbMin;
        mesh->bbMax = bbMax;

        return true;
    }

    bool parseLine(
        const char *c, 
        size_t lineNumber, 
        Mesh *mesh, 
        Vec3f &bbMin, 
        Vec3f &bbMax)
    {
        // Ignore empty lines and comments
        skipWhitespace(c);

        if (*c == '\0' || *c == '#')
            return true;

        // Read the first word
        const char *kwStart = c;
        skipNonWhitespace(c);
        const char *kwEnd = c - 1;

        // Parse statement body
        skipWhitespace(c);

        if (str::compareSlice(kwStart, kwEnd, "v")) {
            Vec3f vert{};

            if (!parseVert(c, lineNumber, vert))
                return false;

            mesh->verts.push_back(vert);

            // Updating the bounding box
            if (vert.x < bbMin.x) bbMin.x = vert.x;
            if (vert.y < bbMin.y) bbMin.y = vert.y;
            if (vert.z < bbMin.z) bbMin.z = vert.z;

            if (vert.x > bbMax.x) bbMax.x = vert.x;
            if (vert.y > bbMax.y) bbMax.y = vert.y;
            if (vert.z > bbMax.z) bbMax.z = vert.z;

            return true;
        }

        if (str::compareSlice(kwStart, kwEnd, "f")) {
            Polygon polygon{};

            if (!parseFace(c, lineNumber, mesh, polygon))
                return false;

            std::vector<Triangle> tris 
                = transform::earClipping(mesh, polygon);

            for (Triangle &triangle : tris)
                mesh->faces.push_back(triangle);

            return true;
        }

        // Skip unsupported/invalid statements
        logger::info(
            "OBJ Parser @ ln %zu: Skipped '%.*s' statement",
            lineNumber,
            kwEnd - kwStart + 1, kwStart
        );

        return true;
    }

    bool parseVert(const char *c, size_t lineNumber, Vec3f &out)
    {
        for (int i = 0; i < 3; i++) {
            const char *numStart = c;
            skipNonWhitespace(c);
            const char *numExpectedEnd = c - 1;

            float result = 0.0f;
            str::ParseError error = str::toFloat(
                numStart, 
                numExpectedEnd,
                result
            );

            if (error != str::ParseError::Good) {
                printVec3fFloatError(error, lineNumber, i, c, numStart);
                return false;
            } 

            switch (i) {
                case 0: out.x = result; break; 
                case 1: out.y = result; break;
                case 2: out.z = result; break;
            }

            skipWhitespace(c);
        }

        if (*c != '\0') {
            logger::info(
                "OBJ Parser @ ln %zu: Discarding everything after the first 3 numbers", 
                lineNumber
            );
        }

        return true;
    }

    bool parseFace(
        const char *c, 
        size_t lineNumber, 
        const Mesh *mesh, 
        Polygon &out)
    {
        if (*c == '\0') {
            logger::error(
                "OBJ Parser @ ln %zu: Expected at least 3 values in one of the following formats: 'v', 'v/vt', 'v//vn', 'v/vt/vn'", 
                lineNumber
            );

            return false;
        }

        out.indices.clear();

        // NOTE: Wavefront OBJ explicitly specifies that the provided 
        // values must be consistent within a single statement (e.g. 
        // statement 'f v//vn v/vt/vn' is illegal, but 'f v//vn v//vn' 
        // is legal). There used to be a check for this, and I used to 
        // emit a warning, but I decided that nobody cares, so ignoring 
        // these inconsistencies is intended behaviour now

        // Reading a polygon
        size_t tupleNumber = 1;

        while (*c != '\0') {
            if (*c == '/') {
                logger::error(
                    "OBJ Parser @ ln %zu: (Tuple %zu) Expected a vertex index", 
                    lineNumber,
                    tupleNumber
                );

                return false;
            }

            // Reading v (this/../..)
            const char *indexStart = c;

            while (!isWhitespaceOrNull(c) && *c != '/')
                c++;

            const char *indexEnd = c - 1;

            size_t index = 0;
            str::ParseError error = parseIndex(
                indexStart, 
                indexEnd, 
                mesh->verts.size(), 
                index
            );

            if (error != str::ParseError::Good) {
                printFaceIndexError(
                    error, 
                    lineNumber, 
                    tupleNumber, 
                    c, 
                    indexStart
                );

                return false;
            }

            out.indices.push_back(index);

            if (*c == '/') {
                logger::info(
                    "OBJ Parser @ ln %zu: (Tuple %zu) Discarding everything after the '/'", 
                    lineNumber,
                    tupleNumber
                );

                skipNonWhitespace(c);
            }

            skipWhitespace(c);
            tupleNumber++;
        }

        if (out.indices.size() < 3) {
            logger::error(
                "OBJ Parser @ ln %zu: Expected at least 3 values in 'v', 'v/vt', 'v//vn' or 'v/vt/vn' format, got %zu", 
                lineNumber,
                out.indices.size()
            );

            return false;
        }

        return true;
    }

// Utility

    // This is kinda bad, but it is better than 
    // to have it inside parseVert or parseFloat
    void printVec3fFloatError(
        str::ParseError error, 
        size_t lineNumber,
        int i, 
        const char *c,
        const char *numStart)
    {
        switch (error) {
        case str::ParseError::Good:
            assert(0 && "Check if error == str::ParseError::Good before calling this!");
            return;

        case str::ParseError::IsNullChar:
            logger::error(
                "OBJ Parser @ ln %zu: Expected 3 or 4 numbers, got %i", 
                lineNumber,
                i
            );
            return;

        case str::ParseError::Overflow:
            logger::error(
                "OBJ Parser @ ln %zu: Magnitude of number '%.*s' is too big", 
                lineNumber,
                c - numStart, numStart
            );
            return;

        case str::ParseError::Underflow:
            logger::error(
                "OBJ Parser @ ln %zu: Magnitude of number '%.*s' is too small", 
                lineNumber,
                c - numStart, numStart
            );
            return;

        case str::ParseError::InvalidConversion:
            logger::error(
                "OBJ Parser @ ln %zu: '%.*s' is not a valid decimal number", 
                lineNumber,
                c - numStart, numStart
            );
            return;

        case str::ParseError::NaN:
            logger::error(
                "OBJ Parser @ ln %zu: NaNs are not supported", 
                lineNumber
            );
            return;

        case str::ParseError::Infinity:
            logger::error(
                "OBJ Parser @ ln %zu: Infinity is not supported", 
                lineNumber
            );
            return;

        default:
            assert(0 && "Unhandled case");
            return;
        }
    }

    // Same as the above function
    void printFaceIndexError(
        str::ParseError error, 
        size_t lineNumber,
        size_t tupleNumber,
        const char *c,
        const char *numStart)
    {
        switch (error) {
        case str::ParseError::Good:
            assert(0 && "Check if error == str::ParseError::Good before calling this!");
            return;

        case str::ParseError::Overflow:
            logger::error(
                "OBJ Parser @ ln %zu: (Tuple %zu) Index '%.*s' is too big of a number", 
                lineNumber,
                tupleNumber,
                c - numStart, numStart
            );
            return;

        case str::ParseError::InvalidConversion:
            logger::error(
                "OBJ Parser @ ln %zu: (Tuple %zu) '%.*s' is not a valid index", 
                lineNumber,
                tupleNumber,
                c - numStart, numStart
            );
            return;

        case str::ParseError::IsZero:
            logger::error(
                "OBJ Parser @ ln %zu: (Tuple %zu) Indices should start with 1, got 0", 
                lineNumber,
                tupleNumber
            );
            return;

        case str::ParseError::OutOfRange:
            logger::error(
                "OBJ Parser @ ln %zu: (Tuple %zu) Index '%.*s' is out of range", 
                lineNumber,
                tupleNumber,
                c - numStart, numStart
            );
            return;

        default:
            assert(0 && "Unhandled case");
            return;
        }
    }

    str::ParseError parseIndex(
        const char *numStart, 
        const char *numExpectedEnd, 
        size_t arraySize,
        size_t &out)
    {
        bool isRelative = false;

        if (*numStart == '-') {
            isRelative = true;
            numStart++;
        }

        size_t index = 0;
        str::ParseError error = str::toSizeT(numStart, numExpectedEnd, index);

        if (error != str::ParseError::Good)
            return error;

        if (index == 0)
            return str::ParseError::IsZero;

        if (index > arraySize)
            return str::ParseError::OutOfRange;

        if (isRelative) {
            index = arraySize - index;
        } else {
            index--; // OBJ has 1 based indices
        }

        out = index;
        return str::ParseError::Good;
    }

    void skipWhitespace(const char *&c)
    {
        while (*c == ' ' 
            || *c == '\t')
            c++;
    }

    void skipNonWhitespace(const char *&c)
    {
        while (*c != ' ' 
            && *c != '\t' 
            && *c != '\0')
            c++;
    }

    bool isWhitespaceOrNull(const char *c)
    {
        return *c == ' ' || *c == '\t' || *c == '\0';
    }
}

