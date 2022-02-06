#include <network/message.h>

#include <cassert>
#include <system_error>

#include <unistd.h>
#include <sys/socket.h>

typedef uint32_t msg_size_t;

MessageSocket::MessageSocket(int sockfd) :
    sockfd(sockfd)
{}
MessageSocket::~MessageSocket() {
    // can't really do much else at this point
    if(close(sockfd) == -1)
        perror("Failed to close socket");
}
void MessageSocket::update() {
    if(!connected) 
        return;

    // Attempt to receive data
    uint8_t buffer[1024];
    ssize_t numReceivedBytes = recv(sockfd, buffer, sizeof buffer, MSG_DONTWAIT);
    switch(numReceivedBytes) {

        case -1:
            if(errno != EAGAIN && errno != EWOULDBLOCK)
                throw std::system_error(errno, std::generic_category(), "recv failed");
            break;
        break;

        case 0:
            // other side disconnected -> stop all socket operations
            connected = false;
            return;

        default:
            rxBuffer.pushNetworkOrder(buffer, static_cast<size_t>(numReceivedBytes));
            break;
    }

        // Attempt to send data
    if(txBuffer.size() > 0) {
        ssize_t numSentBytes = send(sockfd, txBuffer.ptr(), static_cast<ssize_t>(txBuffer.size()), MSG_DONTWAIT|MSG_NOSIGNAL);
        if(numSentBytes > 0) {
            txBuffer.pop(static_cast<size_t>(numSentBytes));
            txBuffer.maybeCompact();
        } else if(numSentBytes == -1)
            switch(errno) {

                case EAGAIN: 
                    break;

                case EPIPE: 
                    connected = false; 
                    return;

                default:
                    throw std::system_error(errno, std::generic_category(), "send failed");
            }
    }
}
bool MessageSocket::isConnected() const {
    return connected;
}
bool MessageSocket::hasMessage() {
    if(rxBuffer.size() < sizeof(msg_size_t))
        return false;
    size_t messageSize = rxBuffer.peek<msg_size_t>();
    return rxBuffer.size() >= sizeof(msg_size_t) + messageSize;
}
RxBuffer MessageSocket::receiveMessage() {

    // we don't bail out early on disconnect
    // because there might still be messages in the buffer 
    // received before other side disconnected 

    assert(hasMessage());

    RxBuffer result;

    size_t messageSize = rxBuffer.peek<msg_size_t>();
    result.pushNetworkOrder(rxBuffer.ptr()+sizeof(msg_size_t), messageSize);
    rxBuffer.pop(sizeof(msg_size_t)+messageSize);

    return result;
}
void MessageSocket::sendMessage(const TxBuffer &message) {
    if(!connected) 
        return;
    txBuffer << static_cast<msg_size_t>(message.size());
    txBuffer.pushNetworkOrder(message.ptr(), message.size());
}

void MessageSocket::waitForMessage(const Duration &timeout) {

    auto deadline = Clock::now() + timeout;

    while(Clock::now() < deadline) {
        update();
        if(hasMessage())
            return;
        sleep(10ms);
    }
    throw TimeoutError();
}