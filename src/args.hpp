#pragma once

#include <string>

#include "utils.hpp"

namespace args {
    int   getVerbosity();
    bool  getFlipY();

    // Windows only flags
    bool  getDetectFontSize();
    float getFontRatio();

    bool read(int argc, char *argv[], std::string &objPath);

    void printHelp();

    void printFloatError(
        str::ParseError error, 
        int i,
        const char *flagStart, 
        const char *flagEnd,
        const char *numStart,
        const char *numEnd);
}
