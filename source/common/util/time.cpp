#include <util/time.h>

TimeoutError::TimeoutError() : std::runtime_error("Timed out.") {}
TimeoutError::TimeoutError(const std::string &what) : std::runtime_error(what) {}