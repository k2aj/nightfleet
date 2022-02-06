#pragma once

#include <cstdint>
#include <vector>

#include <network/txbuffer.h>
#include <network/rxbuffer.h>
#include <util/time.h>

class MessageSocket {

    public:

    /** Creates a new MessageSocket
     *  @param sockfd 
     *      BSD socket file descriptor obtained from socket() (client) or accept() (server).
     *      MessageSocket assumes exclusive ownership of this file descriptor and will close()
     *      it upon being destroyed.
     */
    MessageSocket(int sockfd);
    ~MessageSocket();
    void update();

    /// @returns true if connection is active, false if the other side disconnected.
    bool isConnected() const;
    bool hasMessage();
    RxBuffer receiveMessage();
    void sendMessage(const TxBuffer &message);
    void waitForMessage(const Duration &timeout);

    private:
    TxBuffer txBuffer;
    RxBuffer rxBuffer;
    int sockfd;
    bool connected = true;
};