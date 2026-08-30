#pragma once

#include <chrono>

double updateDeltaTime(
    std::chrono::time_point<std::chrono::steady_clock> &lastFrameTime);

int run(int argc, char *argv[]);
