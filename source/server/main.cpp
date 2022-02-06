#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#include <future>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

#include <scope_guard.h>
#include <network/defaults.h>
#include <network/message.h>

enum class ServerStatus {
    RUNNING,
    SLOW_SHUTDOWN,
    FAST_SHUTDOWN
};

volatile sig_atomic_t caughtSignal = 0;
void signalHandler(int signum) {
    caughtSignal = signum;
}

int createServerSocket(uint16_t port, int maxQueuedConnectionRequests = 16);
void handleSocket(int sock, const std::atomic<ServerStatus> *serverStatus);

int main(int argc, char **argv) {

    std::cerr << "Registering signal handlers." << std::endl;
    signal(SIGINT, &signalHandler);
    signal(SIGQUIT, &signalHandler);

    std::cerr << "Creating server socket on port " << defaultServerPort << std::endl;
    int serverSocket = createServerSocket(defaultServerPort);
    if(serverSocket == -1)
        return EXIT_FAILURE;
    scope_exit(close(serverSocket));

    std::vector<std::future<void>> threads;
    std::atomic<ServerStatus> serverStatus = ServerStatus::RUNNING;

    std::cerr << "Starting main loop" << std::endl;
    while(true) {

        sockaddr_in connectingAddress;
        socklen_t connectingAddressSize = sizeof connectingAddress;
        int newSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&connectingAddress), &connectingAddressSize);

        if(newSocket != -1) {
            std::cerr << "Accepted new connection (sockfd=" << newSocket << ")" << std::endl;
            threads.push_back(std::async(std::launch::async, &handleSocket, newSocket, &serverStatus));
        } else if(errno == EAGAIN || errno == EWOULDBLOCK) {
            std::this_thread::sleep_for(10ms);
        } else {
            perror("Failed to accept socket, shutting down.");
        }

        if(auto signum = caughtSignal) {

            fprintf(stderr, 
                "Interrupted by signal %d (%s), shutting down.\n",
                static_cast<int>(caughtSignal), strsignal(caughtSignal)
            );

            if(signum == SIGQUIT) serverStatus = ServerStatus::SLOW_SHUTDOWN;
            else serverStatus = ServerStatus::FAST_SHUTDOWN;

            // wait for child threads to complete
            for(auto &child : threads)
                child.wait();

            break;
        }
    }

    return EXIT_SUCCESS;
}

void handleSocket(int sockfd, const std::atomic<ServerStatus> *serverStatus) {
    MessageSocket client(sockfd);

    while(
        client.isConnected() &&
        serverStatus->load() == ServerStatus::RUNNING
    ) {

        client.update();

        while(client.hasMessage()) {
            RxBuffer request = client.receiveMessage();
            TxBuffer response;
            response.pushNetworkOrder(request.ptr(), request.size());
            client.sendMessage(response);
        }

        std::this_thread::sleep_for(10ms);
    }

    //socket cleanup handled by MessageSocket
}

int createServerSocket(uint16_t serverPort, int maxQueuedConnectionRequests) {

    int serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == -1) {
        perror("Failed to create socket");
        return -1;
    }

    sockaddr_in address;
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(serverPort);
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serverSocket, (sockaddr *)(&address), sizeof address) == -1) {
        perror("Failed to bind socket");
        close(serverSocket);
        return -1;
    }

    if(listen(serverSocket, maxQueuedConnectionRequests) == -1) {
        perror("Failed to listen on socket");
        close(serverSocket);
        return -1;
    }

    if(fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1) {
        perror("Failed to enable non-blocking operation on socket");
        close(serverSocket);
        return -1;
    }

    return serverSocket;
}