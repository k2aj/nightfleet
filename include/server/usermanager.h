#pragma once

#include <string>
#include <set>
#include <mutex>
#include <network/message.h>

/** Responsible for handling login request & validating user credentials. 
 *  This class is thread-safe.
*/
class UserManager {

    public:

    /** Attempts to accept a login request from the client.
     *  (sends appropriate response back to the client).
     * 
     *  @param s MessageSocket containing the login request and used for sending the response
     *  @param outUsername will contain username of the user after a succesful login attempt
     *  @returns true if login attempt succeeded, false otherwise.
     */
    bool acceptLogin(MessageSocket &s, std::string &outUsername);

    private:

    std::mutex mutex;
    std::set<std::string> activeUsers;
};