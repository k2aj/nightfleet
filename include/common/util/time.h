#pragma once

#include <chrono>
#include <stdexcept>

using namespace std::chrono_literals;

typedef std::chrono::high_resolution_clock Clock;
typedef Clock::duration Duration;
typedef Clock::time_point TimePoint;

class TimeoutError : public std::runtime_error {
    public:
    TimeoutError();
    TimeoutError(const std::string &what);
};

void sleep(const Duration &duration);