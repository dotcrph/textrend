#pragma once

#include <cstddef>
#include <cstring>

#include "math.hpp"

namespace terminal {
    bool initialize();
    void update();
    void cleanup();

    bool pollKey(char key);

    Vec2s getScreenSize();

    void print(const char *string, size_t bytes);

    inline void printLiteral(const char *string)
    {
        size_t size = strlen(string);
        print(string, size);
    }

    void printBuffer(char *buffer, const Vec2s &dimensions);

    bool shouldResizeWindow();
}
