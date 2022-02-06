#pragma once

#include <atomic>
#include <mutex>

#include <usermanager.h>

enum class ServerStatus {
    RUNNING,
    SLOW_SHUTDOWN,
    FAST_SHUTDOWN
};

/** Represents the NightFleet server.
 *  This class is thread-safe.
 */
class Server {
    public:

    UserManager userManager;

    ServerStatus status() const;
    
    void requestShutdown();
    void requestFastShutdown();

    private:
    std::atomic<ServerStatus> _status = ServerStatus::RUNNING;
    std::mutex mutex;
};