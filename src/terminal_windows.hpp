#pragma once

#include <windows.h>

namespace terminal {
    const char *getErrorString(DWORD code);
    void setCursorVisibility(bool visible);
}
