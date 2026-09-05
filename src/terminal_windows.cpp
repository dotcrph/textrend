#include "terminal.hpp"
#include "terminal_windows.hpp"

#include <cassert>
#include <cstring>

#include <stdexcept>
#include <windows.h>

#include "args.hpp"
#include "utils.hpp"
#include "logger.hpp"

namespace terminal {
    HANDLE inHandle  = nullptr;
    DWORD  inOldMode = 0;

    HANDLE outHandle = nullptr;

    bool  windowResized = false;
    COORD bufferCells   = {0, 0};

    // The state of each key, indexed by its ascii value from KEY_EVENT_RECORD
    // The state is a bitmask with the following fields:
    //
    //     0x80 : The key is currently being held
    //     0x40 : The key was being held in the previous frame
    //
    char keys[256] = {};

    constexpr int charToVKey[128] = {
        // NOTE: 0xFF  is a reserved nop value (VK__none_)
        //       0x100 indicates that Shift must be pressed

        // Control characters
        /* 0x00 */ 0xFF,      /* 0x01 */ 0xFF,      /* 0x02 */ 0xFF, 
        /* 0x03 */ 0xFF,      /* 0x04 */ 0xFF,      /* 0x05 */ 0xFF, 
        /* 0x06 */ 0xFF,      /* 0x07 */ 0xFF,      /* 0x08 */ VK_BACK, 
        /* 0x09 */ VK_TAB,    /* 0x0A */ VK_RETURN, /* 0x0B */ 0xFF,
        /* 0x0C */ 0xFF,      /* 0x0D */ 0xFF,      /* 0x0E */ 0xFF, 
        /* 0x0F */ 0xFF,      /* 0x10 */ 0xFF,      /* 0x11 */ 0xFF, 
        /* 0x12 */ 0xFF,      /* 0x13 */ 0xFF,      /* 0x14 */ 0xFF, 
        /* 0x15 */ 0xFF,      /* 0x16 */ 0xFF,      /* 0x17 */ 0xFF, 
        /* 0x18 */ 0xFF,      /* 0x19 */ 0xFF,      /* 0x1A */ 0xFF, 
        /* 0x1B */ VK_ESCAPE, /* 0x1C */ 0xFF,      /* 0x1D */ 0xFF, 
        /* 0x1E */ 0xFF,      /* 0x1F */ 0xFF,

        // Special characters
        /* 0x20   */ VK_SPACE, 
        /* 0x21 ! */ 1             | 0x100,
        /* 0x22 " */ VK_OEM_7      | 0x100,
        /* 0x23 # */ 3             | 0x100,
        /* 0x24 $ */ 4             | 0x100,
        /* 0x25 % */ 5             | 0x100,
        /* 0x26 & */ 7             | 0x100,
        /* 0x27 ' */ VK_OEM_7,     
        /* 0x28 ( */ 9             | 0x100,
        /* 0x29 ) */ 0             | 0x100,
        /* 0x2A * */ 8             | 0x100,
        /* 0x2B + */ VK_OEM_PLUS   | 0x100,
        /* 0x2C , */ VK_OEM_COMMA,
        /* 0x2D - */ VK_OEM_MINUS,
        /* 0x2E . */ VK_OEM_PERIOD,
        /* 0x2F / */ VK_OEM_2,

        // Numbers 0x30-0x39
        VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, 
        VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, 

        // Special characters
        /* 0x3A : */ VK_OEM_1      | 0x100,
        /* 0x3B ; */ VK_OEM_1,
        /* 0x3C < */ VK_OEM_COMMA  | 0x100,
        /* 0x3D = */ VK_OEM_PLUS,
        /* 0x3E > */ VK_OEM_PERIOD | 0x100,
        /* 0x3F ? */ VK_OEM_2      | 0x100,
        /* 0x40 @ */ 2             | 0x100,

        // Uppercase letters 0x41-0x5A
        'A' | 0x100,     'B' | 0x100, 
        'C' | 0x100,     'D' | 0x100, 
        'E' | 0x100,     'F' | 0x100, 
        'G' | 0x100,     'H' | 0x100, 
        'I' | 0x100,     'J' | 0x100, 
        'K' | 0x100,     'L' | 0x100, 
        'M' | 0x100,     'N' | 0x100, 
        'O' | 0x100,     'P' | 0x100, 
        'Q' | 0x100,     'R' | 0x100, 
        'S' | 0x100,     'T' | 0x100, 
        'U' | 0x100,     'V' | 0x100, 
        'W' | 0x100,     'X' | 0x100, 
        'Y' | 0x100,     'Z' | 0x100,

        // Special characters
        /* 0x5B [ */ VK_OEM_4,
        /* 0x5C \ */ VK_OEM_5,
        /* 0x5D ] */ VK_OEM_6,
        /* 0x5E ^ */ 6            | 0x100,
        /* 0x5F _ */ VK_OEM_MINUS | 0x100,
        /* 0x60 ` */ VK_OEM_3,

        // Lowercase letters 0x61-0x7A
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        // Special characters
        /* 0x7B { */ VK_OEM_4 | 0x100,
        /* 0x7C | */ VK_OEM_5 | 0x100,
        /* 0x7D } */ VK_OEM_6 | 0x100,
        /* 0x7E ~ */ VK_OEM_3 | 0x100,

        // Control character
        /* 0x7F */ 0xFF
    };

// Lifetime

    bool initialize()
    {
        BOOL good;

        // Get stdin handle
        inHandle = GetStdHandle(STD_INPUT_HANDLE);

        if (inHandle == INVALID_HANDLE_VALUE) {
            logger::error(
                "Failed to get stdin handle! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        // Enable window messages in stdin
        GetConsoleMode(inHandle, &inOldMode);

        DWORD inMode = inOldMode;

        inMode |= ENABLE_WINDOW_INPUT;
        inMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);

        SetConsoleMode(inHandle, inMode);

        // Create new handle/buffer for output
        outHandle = CreateConsoleScreenBuffer(
            GENERIC_READ    | GENERIC_WRITE, 
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            nullptr, 
            CONSOLE_TEXTMODE_BUFFER, 
            nullptr
        );

        if (outHandle == INVALID_HANDLE_VALUE) {
            logger::error(
                "Failed to create a new screen buffer! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        good = SetConsoleActiveScreenBuffer(outHandle);

        if (!good) {
            logger::error(
                "Failed to switch the active screen buffer! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        // Set the screen buffer size
        CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
        good = GetConsoleScreenBufferInfo(outHandle, &bufferInfo);

        if (!good) {
            logger::error(
                "Failed to get the screen buffer info! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        bufferCells = bufferInfo.dwSize;

        // Hide the cursor
        setCursorVisibility(false);

        return true;
    }

    bool update()
    {
        BOOL good;

        windowResized = false;

        // Update keys

        // NOTE: This call causes the GetKeyboardState to actually work. I do 
        // not know why that is the case. I have found this workaround here:
        // https://dev.to/mirrai/windows-hooks-are-weird-3cld
        GetKeyState(0xFF);

        BYTE rawKeys[256] = {};

        // NOTE: Calling GetKeyboardState isn't particularly good for 
        // performance, but I don't think I have any other choice. I 
        // could just read characters from stdin (as I did with Unix 
        // terminal), but then the movement feels clunky. I also tried 
        // reading keyboard events from stdin and setting up a WndProc,
        // but neither of those worked because of various reasons
        good = GetKeyboardState(rawKeys);

        if (!good) {
            logger::error(
                "Failed to get the keyboard state! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        // PERF: This loop can be simdified
        for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            keys[i]  = (keys[i] & 0x80) >> 1; // Held on previous frame
            keys[i] |= rawKeys[i] & 0x80;     // Held on this frame
        }

        // Check if there are new records in stdin
        //
        // This is neccessary because ReadConsoleInput 
        // blocks if there are no new events
        DWORD events = 0;
        GetNumberOfConsoleInputEvents(inHandle, &events);

        if (events == 0)
            return;

        // Read new records
        static INPUT_RECORD records[128];

        DWORD read = 0;
        good = ReadConsoleInputA(
            inHandle, 
            records, 
            sizeof(records) / sizeof(records[0]), 
            &read
        );

        if (!good) {
            logger::error(
                "Failed to read the console input! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        // Update the window size
        for (DWORD i = 0; i < read; i++) {
            if (records[i].EventType != WINDOW_BUFFER_SIZE_EVENT)
                continue;

            WINDOW_BUFFER_SIZE_RECORD &record 
                = records[i].Event.WindowBufferSizeEvent;

            // FIXME: This does not always work. I assume if the program is 
            // in the middle of something heavy and does not respond to input 
            // or messages then the event just doesn't get sent

            windowResized = true;
            bufferCells    = record.dwSize;
        }

        return true;
    }

    void cleanup()
    {
        // Show the cursor
        setCursorVisibility(true);

        // Switch to the main buffer
        HANDLE defaultOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (defaultOutHandle == INVALID_HANDLE_VALUE) {
            logger::error(
                "Failed to get stdout handle! (%s)", 
                getErrorString(GetLastError())
            );
        }

        BOOL good = SetConsoleActiveScreenBuffer(defaultOutHandle);

        if (!good) {
            logger::error(
                "Failed to switch the active screen buffer! (%s)", 
                getErrorString(GetLastError())
            );
        }

        // Free the alt buffer
        CloseHandle(outHandle);

        // Restore stdin

        // For whatever reason some input appears after exiting
        FlushConsoleInputBuffer(inHandle);
        SetConsoleMode(inHandle, inOldMode);
    }

// IO

    bool getKeyDown(char key)
    {
        assert(key >= 0);

        int  vkey     = charToVKey[key] & 0xFF;
        bool useShift = charToVKey[key] & 0x100;

        bool shift = keys[VK_SHIFT] & 0x80;
        if (!useShift)
            shift = !shift;

        bool heldOnThisFrame = keys[vkey] & 0x80;
        bool heldOnPrevFrame = keys[vkey] & 0x40;

        return heldOnThisFrame && !heldOnPrevFrame && shift;
    }

    bool getKeyHeld(char key)
    {
        assert(key >= 0);

        int  vkey     = charToVKey[key] & 0xFF;
        bool useShift = charToVKey[key] & 0x100;

        bool shift = keys[VK_SHIFT] & 0x80;
        if (!useShift)
            shift = !shift;

        bool held = keys[vkey] & 0x80;

        return held && shift;
    }

// Screen

    bool getScreenSize(Vec2s &cells, Vec2s &px)
    {
        // Set the size in cells
        cells = {
            (size_t)bufferCells.X, 
            (size_t)bufferCells.Y
        };

        // Set the size in pixels
        float fontSizeX = args::getFontRatio();
        float fontSizeY = 1.0f;

        CONSOLE_FONT_INFO font;
        BOOL good = GetCurrentConsoleFont(outHandle, FALSE, &font);

        // TODO: Add a flag to disable auto font size detection
        if (good 
         && args::getDetectFontSize()
         && font.dwFontSize.X != 0 // Most of the terminals just send fallback 
         && font.dwFontSize.Y != 0 // values instead of the actual font size
        ) {
            fontSizeX = font.dwFontSize.X;
            fontSizeY = font.dwFontSize.Y;
        }

        px = {
            (size_t)(cells.x * fontSizeX),
            (size_t)(cells.y * fontSizeY),
        };

        return true;
    }

    bool shouldResizeWindow()
    { 
        return windowResized;
    }

// Memory

    char *getPage()
    {
        LPVOID result = VirtualAlloc(
            NULL, 
            4096, 
            MEM_COMMIT | MEM_RESERVE, 
            PAGE_READWRITE
        );

        if (result == NULL)
            return nullptr;

        return (char *)result;
    }

    void freePage(char *page)
    {
        BOOL result = VirtualFree(page, 0, MEM_RELEASE);
        assert(result);
    }

// Misc

    void resetCursor()
    {
        SetConsoleCursorPosition(outHandle, {0, 0});
    }

    bool print(const char *string, size_t bytes)
    {
        assert(outHandle != nullptr);

        const char *rest = string;

        while (bytes > 0) {
            size_t written = 0;

            BOOL res = WriteConsoleA(
                outHandle, 
                string, 
                bytes, 
                (LPDWORD)&written, 
                nullptr
            );

            if (!res) {
                logger::error(
                    "Call to WriteConsoleA failed! (%s)", 
                    getErrorString(GetLastError())
                );

                return false;
            }

            rest  += written;
            bytes -= written;
        }
    }

    const char *getErrorString(DWORD code)
    {
        static char string[8192] = {};

        assert(code != 0);

        size_t length = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, // Ignored because of the first flag
            code,
            MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
            (LPSTR)string,
            sizeof(string) / sizeof(string[0]),
            nullptr // Ignored because of the last flag
        );

        if (length == 0) {
            const char *errorCode = str::quickFormat(
                "Failed to generate an error string (tried to generate a string for 0x%x, failed with 0x%x)\n", 
                code, GetLastError()
            );

            throw std::runtime_error(errorCode);
        }

        // Removing a newline character. Subtracting 2 because on Windows 
        // a newline is \r\n (I wonder why do I even have to do this ._.)
        string[length - 2] = '\0';
        return string;
    }

    void setCursorVisibility(bool visible)
    {
        CONSOLE_CURSOR_INFO cursor;
        GetConsoleCursorInfo(outHandle, &cursor);
        cursor.bVisible = visible;
        SetConsoleCursorInfo(outHandle, &cursor);
    }
}
