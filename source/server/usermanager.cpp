#include <usermanager.h>

#include <network/protocol.h>

bool UserManager::acceptLogin(MessageSocket &s, std::string &outUsername) {

    if(!s.hasMessage())
        return false;

    auto request = s.receiveMessage();
    LoginRequest credentials;

    // Parse the request
    try {
        auto requestType = request.read<MessageType>();

        if(requestType != MessageType::LOGIN_REQUEST)
            throw ProtocolError("Expected login request.");
        
        credentials = request.read<LoginRequest>();
        if(request.size() > 0)
            throw ProtocolError("Response too long.");

    } catch (const std::out_of_range &) {
        throw ProtocolError("Response too short.");
    }

    // Prepare the response
    TxBuffer response;
    response << MessageType::LOGIN_RESPONSE;
    bool result;

    {   // perform the actual login operation
        // (critical section)
        
        std::scoped_lock lk(mutex);

        if(activeUsers.find(credentials.username) == activeUsers.end()) {
            response << LoginResponse::OK;
            outUsername = credentials.username;
            activeUsers.insert(credentials.username);
            result = true;
        } else {
            response << LoginResponse::E_ALREADY_LOGGED_IN;
            result = false;
        }
    }
    s.sendMessage(response);
    return result;
}