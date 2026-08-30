#include "args.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#include "utils.hpp"
#include "logger.hpp"

namespace args {
    int verbosity = 2; // 0 = Silent
                       // 1 = Errors
                       // 2 = Errors + warnings
                       // 3 = Errors + warnings + Info

    int getVerbosity() { return verbosity; }

    bool flipY = false;
    bool getFlipY() { return flipY; }

    // Windows only flags
    bool detectFontSize = true;
    bool getDetectFontSize() { return detectFontSize; }

    // NOTE: The default is the ratio of a single glyph in 
    // Consolas, which likely is different in other fonts
    float fontRatio = 0.45f;
    float getFontRatio() { return fontRatio; }

    bool read(int argc, char *argv[], std::string &objPath)
    {
        if (argc <= 1 
         || strcmp(argv[1], "-h") == 0
         || strcmp(argv[1], "--help") == 0
        ) {
            printHelp();
            return false;
        }

        objPath = argv[1];

        for (int i = 2; i < argc; i++) {
            const char *arg = argv[i];

            const char *flagStart = arg;
            const char *flagEnd   = arg;

            // Getting only the flag name (for constructs like --flag=value)
            while (*flagEnd != '\0' && *flagEnd != '=')
                flagEnd++;

            flagEnd--;

            if (flagEnd - flagStart == 0) {
                logger::warning("Argument #%d is invalid, ignoring", i - 1);
                continue;
            }

            switch (str::djb2(flagStart, flagEnd)) {
                case str::djb2("-v"):
                case str::djb2("--verbosity"): 
                {
                    const char *eq    = flagEnd + 1;
                    const char *digit = flagEnd + 2;

                    if (*eq == '\0' || *digit == '\0') {
                        logger::warning(
                            "Argument #%d (%.*s): A level (0/1/2/3) must be specified after an '=' (i.e. -v=0); ignoring", 
                            i - 1,               // %d
                            flagEnd - flagStart, // %.*s
                            flagStart            // %.*s
                        );

                        continue;
                    }

                    if (*(digit + 1) != '\0' || *digit < '0' || *digit > '3') {
                        logger::warning(
                            "Argument #%d (%.*s): Invalid level specified, can only accept 0/1/2/3; ignoring", 
                            i - 1,               // %d
                            flagEnd - flagStart, // %.*s
                            flagStart            // %.*s
                        );

                        continue;
                    }

                    verbosity = *digit - '0';
                } break;

                case str::djb2("-y"):
                case str::djb2("--flip-y"): 
                {
                    flipY = true;

                    char nextChar = *(flagEnd + 1);

                    if (nextChar != '\0') {
                        logger::warning(
                            "Argument #%d (%.*s): Ignoring everything after '%c'", 
                            i - 1,               // %d
                            flagEnd - flagStart, // %.*s
                            flagStart,           // %.*s
                            nextChar             // %c
                        );
                    }
                } break;

                case str::djb2("--no-detect-font"): 
                case str::djb2("--no-detect-font-size"): 
                {
                    detectFontSize = false;

                    char nextChar = *(flagEnd + 1);

                    if (nextChar != '\0') {
                        logger::warning(
                            "Argument #%d (%.*s): Ignoring everything after '%c'", 
                            i - 1,               // %d
                            flagEnd - flagStart, // %.*s
                            flagStart,           // %.*s
                            nextChar             // %c
                        );
                    }
                } break;

                case str::djb2("--font"): 
                case str::djb2("--font-ratio"): 
                {
                    const char *eq       = flagEnd + 1;
                    const char *numStart = flagEnd + 2;

                    if (*eq == '\0' || *numStart == '\0') {
                        logger::warning(
                            "Argument #%d (%.*s): A decimal number must be specified after an '=' (i.e. -f=0.5); ignoring", 
                            i - 1,               // %d
                            flagEnd - flagStart, // %.*s
                            flagStart            // %.*s
                        );

                        continue;
                    }

                    const char *numEnd = numStart;

                    while (*numEnd != '\0')
                        numEnd++;

                    numEnd--;

                    float ratio = 0;
                    str::ParseError error
                        = str::toFloat(numStart, numEnd, ratio);

                    if (error != str::ParseError::Good) {
                        printFloatError(
                            error, 
                            i, 
                            flagStart, 
                            flagEnd, 
                            numStart, 
                            numEnd
                        );

                        continue;
                    }

                    fontRatio = ratio;
                } break;

                default:
                {
                    logger::warning(
                        "Argument #%d: Unknown argument '%.*s', ignoring", 
                        i - 1,               // %d
                        flagEnd - flagStart, // %.*s
                        flagStart            // %.*s
                    );
                } break;
            }
        }

        return true;
    }

    void printHelp()
    {
        puts(
"Usage: textrend [filename.obj] [-args]                                     \n"
"                                                                           \n"
"Controls:                                                                  \n"
"        WASD         : Move                                                \n"
"        OKL; or 8426 : Look                                                \n"
"                                                                           \n"
"        [ and ]      : -/+ movement speed                                  \n"
"        { and }      : -/+ rotation speed                                  \n"
"        - and +      : Zoom out/in                                         \n"
"                                                                           \n"
"        z/x/c        : Toggle vertices/edges/faces                         \n"
"        b            : Toggle backface culling                             \n"
"                                                                           \n"
"        ESC          : Exit                                                \n"
"Args:                                                                      \n"
"        -v --verbosity        : Set verbosity level                        \n"
"            -v=0              :   Silent                                   \n"
"            -v=1              :   Errors only                              \n"
"            -v=2              :   Errors and warnings (default)            \n"
"            -v=3              :   Full                                     \n"
"                                                                           \n"
"        -y --flip-y           : Flip camera Y rotation                     \n"
"                                                                           \n"
"        --no-detect-font      : Disable font size detection                \n"
"        --no-detect-font-size :   Used on Windows if your terminal         \n"
"                              :   reports incorrect font information       \n"
"                                                                           \n"
"        --font --font-ratio   : Set fallback font width to height ratio    \n"
"            --font=0.45       :   Default (Consolas 11pt)                  \n"
        );
    }

    void printFloatError(
        str::ParseError error, 
        int i,
        const char *flagStart, 
        const char *flagEnd,
        const char *numStart,
        const char *numEnd)
    {
        switch (error) {
        case str::ParseError::Good:
            assert(0 && "Check if error == str::ParseError::Good before calling this!");
            return;

        case str::ParseError::IsNullChar:
            logger::warning(
                "Argument #%d (%.*s): Expected a decimal number; ignoring", 
                i - 1,               // %d
                flagEnd - flagStart, // %.*s
                flagStart            // %.*s
            );
            return;

        case str::ParseError::Overflow:
            logger::warning(
                "Argument #%d (%.*s): Magnitude of number '%.*s' is too big; ignoring", 
                i - 1,               // %d
                flagEnd - flagStart, // 1st %.*s
                flagStart,           // 1st %.*s
                numEnd - numStart,   // 2nd %.*s
                numStart             // 2nd %.*s
            );
            return;

        case str::ParseError::Underflow:
            logger::warning(
                "Argument #%d (%.*s): Magnitude of number '%.*s' is too small; ignoring", 
                i - 1,               // %d
                flagEnd - flagStart, // 1st %.*s
                flagStart,           // 1st %.*s
                numEnd - numStart,   // 2nd %.*s
                numStart             // 2nd %.*s
            );
            return;

        case str::ParseError::InvalidConversion:
        case str::ParseError::NaN:
        case str::ParseError::Infinity:
            logger::warning(
                "Argument #%d (%.*s): '%.*s' is not a valid decimal number; ignoring", 
                i - 1,               // %d
                flagEnd - flagStart, // 1st %.*s
                flagStart,           // 1st %.*s
                numEnd - numStart,   // 2nd %.*s
                numStart             // 2nd %.*s
            );
            return;

        default:
            assert(0 && "Unhandled case");
            return;
        }
    }
}
