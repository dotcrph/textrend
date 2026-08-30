#include "screen.hpp"

#include <cassert>
#include <cfloat>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "math.hpp"
#include "terminal.hpp"

namespace screen {
    size_t bufferCount; 

    Vec2s bufferDimensions; // Exclusive (last index + 1)
    Vec2s getBufferDimensions() { return bufferDimensions; }

    char  *frameBuffer = nullptr;
    float *depthBuffer = nullptr;

    float widthDivHeight;
    float getWidthDivHeight() { return widthDivHeight; }

    bool initialize()
    {
        Vec2s bufferSizePx = Vec2s::zero();
        bool good = terminal::getScreenSize(bufferDimensions, bufferSizePx);

        if (!good)
            return false;

        widthDivHeight = (float)bufferSizePx.x / bufferSizePx.y;

        // Allocate buffers
        bufferCount = bufferDimensions.x * bufferDimensions.y;

        frameBuffer = new char [bufferCount];
        depthBuffer = new float[bufferCount];

        clear();
        return true;
    }

    void cleanup()
    {
        delete[] frameBuffer;
        delete[] depthBuffer;
    }

    void clear()
    {
        memset(frameBuffer, ' ', bufferCount);

        for (size_t i = 0; i < bufferCount; i++)
            depthBuffer[i] = FLT_MAX;
    }

    bool print()
    {
        terminal::resetCursor();
        if (!terminal::print(frameBuffer, bufferDimensions.x))
            return false;

        for (size_t row = 1; row < bufferDimensions.y; row++) {
            char *oneBeforeFirst = frameBuffer 
                                 + bufferDimensions.x * (row - 1) 
                                 + bufferDimensions.x - 1;

            // This is a hack, but I do not want to 
            // allocate a new string just to insert newlines
            char originalChar = *oneBeforeFirst;
            *oneBeforeFirst = '\n';

            if (!terminal::print(oneBeforeFirst, bufferDimensions.x + 1))
                return false;

            *oneBeforeFirst = originalChar;
        }

        return true;
    }

    size_t cellToIndex(Vec2s cell)
    {
        return cellToIndex(cell.x, cell.y);
    }

    size_t cellToIndex(size_t x, size_t y)
    {
        assert(x < bufferDimensions.x 
            && y < bufferDimensions.y);

        return y * bufferDimensions.x + x;
    }

    void draw(Vec2s cell, char c)
    {
        draw(cell.x, cell.y, c);
    }

    void draw(size_t x, size_t y, char c)
    {
        frameBuffer[cellToIndex(x, y)] = c;
    }

    void drawDepthTest(Vec2s cell, char c, float depth)
    {
        drawDepthTest(cell.x, cell.y, c, depth);
    }

    void drawDepthTest(size_t x, size_t y, char c, float depth)
    {
        if (depth >= depthBuffer[cellToIndex(x, y)])
            return;

        frameBuffer[cellToIndex(x, y)] = c;
        depthBuffer[cellToIndex(x, y)] = depth;
    }

    void drawText(size_t originX, size_t originY, const char *text)
    {
        size_t x = originX;
        size_t y = originY;

        while (*text != '\0') {
            char c = *text;

            if (c == '\n' || x > bufferDimensions.x) {
                x = originX;
                y++;
            }

            if (y > bufferDimensions.y)
                return;

            // If the character is printable
            if (c >= 32 && c <= 126) {
                draw(x, y, c);
                x++;
            }

            text++;
        }
    }
}
