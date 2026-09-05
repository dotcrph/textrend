#pragma once

#include <cstddef>
#include <cstring>

#include "math.hpp"

namespace terminal {
    bool initialize();
    bool update();
    void cleanup();

    bool getKeyDown(char key);
    bool getKeyHeld(char key);

    bool getScreenSize(Vec2s &cells, Vec2s &px);
    bool shouldResizeWindow();

    char *getPage();
    void freePage(char *page);

    bool isTerminal(FILE *file);

    void resetCursor();

    bool print(const char *string, size_t bytes);

    inline bool printLiteral(const char *string)
    {
        size_t size = strlen(string);
        return print(string, size);
    }
}
