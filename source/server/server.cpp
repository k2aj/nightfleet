#include <server.h>

ServerStatus Server::status() const {
    return _status;
}
    
void Server::requestShutdown() {
    if(_status < ServerStatus::SLOW_SHUTDOWN)
        _status = ServerStatus::SLOW_SHUTDOWN;
}
void Server::requestFastShutdown() {
    if(_status < ServerStatus::FAST_SHUTDOWN)
        _status = ServerStatus::FAST_SHUTDOWN;
}