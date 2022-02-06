#include <clientlogin.h>

LoginResponse tryLogin(MessageSocket &s, const LoginRequest &credentials) {

    TxBuffer request;
    request << MessageType::LOGIN_REQUEST << credentials;
    s.sendMessage(request);
    s.waitForMessage(5s);

    RxBuffer response = s.receiveMessage();

    try {
        auto responseType = response.read<MessageType>();

        if(responseType != MessageType::LOGIN_RESPONSE)
            throw ProtocolError("Expected login response.");

        LoginResponse result;
        response >> result;
        if(response.size() > 0)
            throw ProtocolError("Response too long!");

        return result;

    } catch (const std::out_of_range &) {
        throw ProtocolError("Response too short!");
    }
}