#pragma once

namespace terminal {
    bool pollKey(char key);
    void readInput();
    void sigwinchHandler(int n);
}
