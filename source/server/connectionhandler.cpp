#include <connectionhandler.h>

#include <iostream>

#include <network/message.h>
#include <network/rxbuffer.h>
#include <network/txbuffer.h>
#include <network/protocol.h>
#include <util/time.h>
#include <scope_guard.h>

void handleConnection(int sockfd, Server *server) {

    MessageSocket client(sockfd);
    std::cerr << "Connection accepted (sockfd="<<sockfd<<")" << std::endl;
    scope_exit(std::cerr << "Connection terminated (sockfd="<<sockfd<<")"<<std::endl);

    if(!performVersionHandshake(client, 5s))
        return;

    std::string username;
    do {
        if(server->status() != ServerStatus::RUNNING)
            return;
        client.update();
        sleep(100ms);
    } while(!server->userManager.acceptLogin(client, username));

    std::cerr << "Login: " << username << std::endl;
    scope_exit(
        std::cerr << "Logout: " << username << std::endl; 
        server->userManager.logout(username)
    );

    while(client.isConnected()) {

        if(server->status() != ServerStatus::RUNNING)
            return;

        client.update();

        while(client.hasMessage()) {
            RxBuffer request = client.receiveMessage();
            request.read<MessageType>();
            TxBuffer response;
            response << MessageType::ALERT;
            response.pushNetworkOrder(request.ptr(), request.size());
            client.sendMessage(response);
        }

        sleep(10ms);
    }
}

