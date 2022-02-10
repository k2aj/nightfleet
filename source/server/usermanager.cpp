#include <usermanager.h>

#include <cassert>
#include <network/protocol.h>

LoginResponse UserManager::tryLogin(const LoginRequest &credentials) {

    std::scoped_lock lk(mutex);

    if(activeUsers.find(credentials.username) == activeUsers.end()) {
        activeUsers.insert(credentials.username);
        return LoginResponse::OK;
    } else
        return LoginResponse::E_ALREADY_LOGGED_IN;
}

void UserManager::logout(const std::string &username) {
    std::scoped_lock lk(mutex);
    assert(activeUsers.find(username) != activeUsers.end());
    activeUsers.erase(username);
}