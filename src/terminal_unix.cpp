#include "terminal.hpp"
#include "terminal_unix.hpp"

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "math.hpp"
#include "logger.hpp"

namespace terminal {
    pollfd inputPollFD;
    char keys[128] = {};

    int outputFD = -1;

    bool terminalFlagsInitialized = false;
    termios originalTerminalFlags;

    bool windowsResized = false;

// Lifetime

    bool initialize()
    {
        // Register sigwinch handler
        struct sigaction action;

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        action.sa_handler = sigwinchHandler;

        int res = sigaction(SIGWINCH, &action, nullptr);

        if (res == -1) {
            logger::error(
                "Failed to register SIGWINCH handler! (errno %d: %s)", 
                errno, strerror(errno)
            );

            return false;
        }

        // Disable input buffering and echoing
        tcgetattr(STDIN_FILENO, &originalTerminalFlags);
        termios terminalFlags = originalTerminalFlags;
        terminalFlags.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminalFlags);

        terminalFlagsInitialized = true;

        // Create a pollfd for input 
        inputPollFD.fd     = STDIN_FILENO;
        inputPollFD.events = POLLIN;

        // Open /dev/tty for output

        // I am doing this in case someone would want to 
        // pipe the output to a file to see the log messages
        outputFD = open("/dev/tty", O_RDWR | O_APPEND);

        if (outputFD == -1) {
            logger::error(
                "Failed to open '/dev/tty' for output! (errno %d: %s)", 
                errno, strerror(errno)
            );

            return false;
        }
        
        printLiteral(
            "\x1b[?25l"   // Hide the cursor
            "\x1b[?1049h" // Switch to the alternate screen buffer
        );
        
        return true;
    }

    bool update()
    {
        windowsResized = false;

        // Read input from stdin
        memset(keys, 0, sizeof(keys));

        // FIXME: Google says that using poll 
        // with /dev/tty on Mac is unreliable
        int result = poll(&inputPollFD, 1, 0);

        // Ignoring errors
        if (result == -1)
            return true; // TODO: This should return false

        // Timeout
        if (result == 0)
            return true;

        // Reading stdin if there is data to read
        if (inputPollFD.revents & POLLIN)
            read(STDIN_FILENO, &keys, sizeof(keys));

        return true;
    }

    void cleanup()
    {
        // Restore attributes and close input fd
        if (terminalFlagsInitialized)
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerminalFlags);

        // Close output fd
        if (outputFD != -1) {
            printLiteral(
                "\x1b[?1049l" // Switch to the main buffer
                "\x1b[?25h"   // Show the cursor
            );

            close(outputFD);
        }
    }

// IO

    // NOTE: This is obviously dumb, but doing this properly 
    // would require splitting this file into separate Linux/MacOS 
    // implementations. This provides little benefit, and reading 
    // from /dev/tty is just good enough for my use case.
    bool getKeyDown(char key)
    {
        return pollKey(key);
    }

    bool getKeyHeld(char key)
    {
        return pollKey(key);
    }

    bool pollKey(char key)
    {
        for (int i = 0; i < sizeof(keys); i++)
            if (keys[i] == key)
                return true;

        return false;
    }

// Screen

    bool getScreenSize(Vec2s &cells, Vec2s &px)
    {
        assert(outputFD != -1);

        winsize window;
        int ioctlResult = ioctl(outputFD, TIOCGWINSZ, &window);

        if (ioctlResult == -1) {
            logger::error(
                "Call to ioctl() failed! (errno %d: %s)", 
                errno, strerror(errno)
            );

            return false;
        }

        cells = {window.ws_col,    window.ws_row};
        px    = {window.ws_xpixel, window.ws_ypixel};

        return true;
    }

    bool shouldResizeWindow()
    { 
        return windowsResized;
    }

    void sigwinchHandler(int n)
    {
        windowsResized = true;
    }

// Memory

    char *getPage()
    {
        void *result = mmap(
            nullptr, 
            4096, 
            PROT_READ | PROT_WRITE, 
            MAP_PRIVATE | MAP_ANONYMOUS, 
            -1, 
            0
        );

        if (result == MAP_FAILED)
            return nullptr;
        
        return (char *)result;
    }

    void freePage(char *page)
    {
        int result = munmap(page, 4096);
        assert(result == 0);
    }

// Misc

    bool isTerminal(FILE *file)
    {
        return isatty(fileno(file));
    }

    void resetCursor()
    {
        printLiteral("\x1b[H");
    }

    bool print(const char *string, size_t bytes)
    {
        assert(outputFD != -1);

        const char *rest = string;

        while (bytes > 0) {
            ssize_t written = write(outputFD, rest, bytes);

            if (written == -1) {
                if (errno == EINTR)
                    continue;

                logger::error(
                    "Failed to write to /dev/tty! (errno %d: %s)", 
                    errno, strerror(errno)
                );

                return false;
            }

            rest  += written;
            bytes -= written;
        }

        return true;
    }
}
