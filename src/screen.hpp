#pragma once

#include "math.hpp"

namespace screen {
    bool initialize();
    void cleanup();

    void clear();
    void print();

    Vec2s getBufferDimensions();
    float getWidthDivHeight();

    size_t cellToIndex(Vec2s cell);
    size_t cellToIndex(size_t x, size_t y);

    void draw(Vec2s cell, char c);
    void draw(size_t x, size_t y, char c);

    void drawDepthTest(Vec2s cell, char c, float depth);
    void drawDepthTest(size_t x, size_t y, char c, float depth);

    void drawText(size_t originX, size_t originY, const char *text);
}
