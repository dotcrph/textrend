#include "terminal.hpp"
#include "terminal_unix.hpp"

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sstream>
#include <stdexcept>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "utils.hpp"
#include "math.hpp"

namespace terminal {
    int inputFD              = -1;
    int originalInputFDFlags =  0;

    pollfd inputPollFD;
    char keys[16] = {};

    int outputFD = -1;

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

        // Open /dev/tty for input

        // I am not using STDIN_FILENO because it shares the same 
        // file with STDOUT_FILENO, and since I set O_NONBLOCK 
        // this results in writes to stdout being non-blocking
        inputFD = open("/dev/tty", O_RDONLY);

        if (inputFD == -1) {
            logger::error(
                "Failed to open '/dev/tty' for input! (errno %d: %s)", 
                errno, strerror(errno)
            );

            return false;
        }

        // Disable input buffering and echoing
        tcgetattr(inputFD, &originalTerminalFlags);
        termios terminalFlags = originalTerminalFlags;
        terminalFlags.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(inputFD, TCSAFLUSH, &terminalFlags);

        originalInputFDFlags = fcntl(inputFD, F_GETFL, 0);
        fcntl(inputFD, F_SETFL, originalInputFDFlags | O_NONBLOCK);

        // Create a pollfd for input 
        inputPollFD.fd     = inputFD;
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
            "\x1b[?47h" // Switch to the alternate screen buffer
            "\x1b[?25l" // Hide the cursor
            "\x1b[s"    // Save the position of the cursor
        );
        
        return true;
    }

    void update()
    {
        windowsResized = false;

        // Read input from stdin
        memset(keys, 0, sizeof(keys));

        // FIXME: Google says that using poll 
        // with /dev/tty on Mac is unreliable
        int result = poll(&inputPollFD, 1, 0);

        // Ignoring errors
        if (result == -1)
            return;

        // Timeout
        if (result == 0)
            return;

        // Reading stdin if there is data to read
        if (inputPollFD.revents & POLLIN)
            read(inputFD, &keys, sizeof(keys));
    }

    void cleanup()
    {
        // Restore attributes and close input fd
        if (inputFD != -1) {
            tcsetattr(inputFD, TCSAFLUSH, &originalTerminalFlags);
            fcntl(inputFD, F_SETFL, originalInputFDFlags);
            close(inputFD);
        }

        // Close output fd
        if (outputFD != -1) {
            printLiteral(
                "\x1b[u"    // Restore the position of the curspr
                "\x1b[?25h" // Show the cursor
                "\x1b[?47l" // Switch to the main buffer
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

// Misc

    void resetCursor()
    {
        printLiteral("\x1b[H");
    }

    void print(const char *string, size_t bytes)
    {
        assert(outputFD != -1);

        const char *rest = string;

        while (bytes > 0) {
            ssize_t written = write(outputFD, rest, bytes);

            if (written == -1) {
                if (errno == EINTR)
                    continue;

                std::stringstream errorMsg;
                errorMsg << "Failed to write to /dev/tty! (errno " 
                         << errno << ": " << strerror(errno) << ")";

                throw std::runtime_error(errorMsg.str());
            }

            rest  += written;
            bytes -= written;
        }
    }
}
