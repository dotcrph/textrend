#pragma once

#include <cstddef>

#include "math.hpp"

namespace terminal {
    bool initialize();
    void update();
    void cleanup();

    void readInput();
    bool pollKey(char key);

    Vec2s getScreenSize();

    void print(const char *string, size_t bytes);
    void printLiteral(const char *string);
    void printBuffer(char *buffer, const Vec2s &dimensions);

    bool shouldResizeWindow();
}
