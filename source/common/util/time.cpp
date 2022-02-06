#include <util/time.h>

#include <thread>

TimeoutError::TimeoutError() : std::runtime_error("Timed out.") {}
TimeoutError::TimeoutError(const std::string &what) : std::runtime_error(what) {}

void sleep(const Duration &duration) {
    std::this_thread::sleep_for(duration);
}