#pragma once

#include <cstddef>
#include <cstring>

#include "math.hpp"

namespace terminal {
    bool initialize();
    void update();
    void cleanup();

    bool getKeyDown(char key);
    bool getKeyHeld(char key);

    bool getScreenSize(Vec2s &cells, Vec2s &px);
    bool shouldResizeWindow();

    void resetCursor();

    void print(const char *string, size_t bytes);

    inline void printLiteral(const char *string)
    {
        size_t size = strlen(string);
        print(string, size);
    }
}
