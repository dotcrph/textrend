#include "terminal.hpp"
#include "terminal_windows.hpp"

#include <cassert>
#include <cstring>

#include <stdexcept>
#include <sstream>
#include <wincontypes.h>
#include <windows.h>

#include "utils.hpp"

namespace terminal {
    HANDLE inHandle  = nullptr;
    DWORD  inOldMode = 0;

    HANDLE outHandle     = nullptr;
    Vec2s  outDimensions = Vec2s::zero();

    bool windowsResized = false;

    static constexpr int charToVKey[128] = {
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
        /* 0x5E ^ */ 6             | 0x100,
        /* 0x5F _ */ VK_OEM_MINUS  | 0x100,
        /* 0x60 ` */ VK_OEM_3,

        // Lowercase letters 0x61-0x7A
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        // Special characters
        /* 0x7B { */ VK_OEM_4      | 0x100,
        /* 0x7C | */ VK_OEM_5      | 0x100,
        /* 0x7D } */ VK_OEM_6      | 0x100,
        /* 0x7E ~ */ VK_OEM_3      | 0x100,

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

        // Initialize the outDimensions
        CONSOLE_SCREEN_BUFFER_INFO screenInfo;
        good = GetConsoleScreenBufferInfo(outHandle, &screenInfo);

        if (!good) {
            logger::error(
                "Failed to get the screen buffer info! (%s)", 
                getErrorString(GetLastError())
            );

            return false;
        }

        outDimensions = {
            (size_t)screenInfo.dwSize.X, 
            (size_t)screenInfo.dwSize.Y
        };

        // Hide the cursor
        setCursorVisibility(false);

        return true;
    }

    void update()
    {
        windowsResized = false;

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
        bool good = ReadConsoleInputA(
            inHandle, 
            records, 
            sizeof(records) / sizeof(records[0]), 
            &read
        );

        if (!good)
            return;

        // Update the window size
        // TODO: For whatever reason this does not always work?
        for (DWORD i = 0; i < read; i++) {
            INPUT_RECORD &record = records[i];

            if (record.EventType != WINDOW_BUFFER_SIZE_EVENT)
                continue;

            COORD newDimensions = record.Event.WindowBufferSizeEvent.dwSize;

            outDimensions = {
                (size_t)newDimensions.X, 
                (size_t)newDimensions.Y
            };

            windowsResized = true;
        }
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

    bool pollKey(char key)
    {
        int vKey  = charToVKey[key] & 0xFF;
        int shift = charToVKey[key] & 0x100;

        bool keyPressed   = GetAsyncKeyState(vKey);
        bool shiftPressed = GetAsyncKeyState(VK_SHIFT);

        if (!shift)
            shiftPressed = !shiftPressed;

        return keyPressed && shiftPressed && !GetAsyncKeyState(VK_CONTROL);
    }

    Vec2s getScreenSize()
    {
        return outDimensions;
    }

// Print

    void print(const char *string, size_t bytes)
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
                std::stringstream errorMsg;
                errorMsg << "Call to WriteConsoleA failed! (" 
                         << getErrorString(GetLastError()) << ")";

                throw std::runtime_error(errorMsg.str());
            }

            rest  += written;
            bytes -= written;
        }
    }

    void printBuffer(char *buffer, const Vec2s &dimensions)
    {
        SetConsoleCursorPosition(outHandle, {0, 0});
        print(buffer, dimensions.x);

        for (size_t row = 1; row < dimensions.y; row++) {
            char *oneBeforeStart = buffer + dimensions.x * (row - 1) 
                                          + dimensions.x - 1;

            // This is a hack, but I do not want to 
            // allocate a new string just to insert newlines
            char originalChar = *oneBeforeStart;
            *oneBeforeStart = '\n';
            print(oneBeforeStart, dimensions.x + 1);
            *oneBeforeStart = originalChar;
        }
    }

// Window resizing

    bool shouldResizeWindow()
    { 
        return windowsResized;
    }

// Misc

    const char *getErrorString(DWORD code)
    {
        static char string[8192] = {};

        assert(code != 0);

        size_t length = FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, // Ignored because of the first flag
            code,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)string,
            0,      // Ignored because I'm not using the allocate flag
            nullptr // Ignored because of the last flag
        );

        assert(length > 0);

        #ifdef UNICODE
            // Converting from UTF-16 to UTF-8
            //
            // If we're using ANSI then LPTSTR == char *, 
            // so we don't need to convert

            int newLength = WideCharToMultiByte(
                CP_UTF8, 
                0, 
                (LPCWCH)string,
                -1,
                nullptr,
                0,
                nullptr,
                nullptr
            );

            if (newLength > sizeof(string))
                throw std::runtime_error("Error message too large");

            static char utf8String[sizeof(string)] = {};

            WideCharToMultiByte(
                CP_UTF8, 
                0, 
                (LPCWCH)string,
                -1,
                utf8String,
                newLength,
                nullptr,
                nullptr
            );

            memcpy(string, utf8String, newLength);
        #endif

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
