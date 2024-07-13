#pragma once

#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

public:
    double elapsedSeconds() const {
        return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
    }
};
