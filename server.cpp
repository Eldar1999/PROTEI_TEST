#include "myLib.h"

bool toClose = false;

void closeHandler(int signo) {
    std::cout << "Signal " << signo << " received" << std::endl;
    fflush(stdout);
    toClose = true;
}

int main() {
    struct sigaction act{};
    act.sa_handler = closeHandler;
    sigaction(SIGKILL, &act, nullptr);
    sigaction(SIGTERM, &act, nullptr);
    timeval t{};
    t.tv_sec = 1;
    t.tv_usec = 0;

    sockaddr_in saTCP{}, saUDP{};
    saTCP.sin_family = AF_INET;
    saTCP.sin_port = htons(1100);
    saTCP.sin_addr.s_addr = htonl(INADDR_ANY);

    saUDP.sin_family = AF_INET;
    saUDP.sin_port = htons(1101);
    saUDP.sin_addr.s_addr = htonl(INADDR_ANY);

    int socketTCP = raiseTCPServer(saTCP, 1);
    if (check(socketTCP, "Ubable to get TCP socket") == -1) {
        return 1;
    }
    int socketUDP = raiseUDPServer(saUDP, 1);
    if (check(socketUDP, "Ubable to get UDP socket") == -1) {
        return 1;
    }

    fd_set current_sockets, ready_sockets, write_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(socketTCP, &current_sockets);
    FD_SET(socketUDP, &current_sockets);


    for (;;) {
        usleep(1000);
        //std::cout<<".";

        ready_sockets = current_sockets;
        write_sockets = current_sockets;

        if (toClose) {
            std::cout << "Received close signal! Socket`s closing!" << std::endl;
            for (int i = 0; i < FD_SETSIZE; i++) {
                if (FD_ISSET(i, &ready_sockets)) {
                    std::cout << "Close: " << i << std::endl;
                    close(i);
                    FD_CLR(i, &ready_sockets);
                }
            }
            std::cout << "Done!" << std::endl;
            fflush(stdout);
            break;
        }

        check(select(FD_SETSIZE, &ready_sockets, &write_sockets, nullptr, &t), "select error");
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == socketTCP) {
                    std::cout << "TCP connection received" << std::endl;
                    int ConnectFD = accept(socketTCP, nullptr, nullptr);

                    if (check(ConnectFD, "accept failed") == -1) {
                        close(socketTCP);
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(ConnectFD, &current_sockets);
                } else if (i == socketUDP) {
                    std::cout << "UDP message received" << std::endl;
                    std::cout << "Get message...";
                    fflush(stdout);
                    Message message = getMessage(i, &saUDP);
                    fflush(stdin);
                    std::cout << message << std::endl;
                    std::vector<int> nums = getNums(message);
                    std::cout << nums.size() << std::endl;
                    if (!nums.empty()) {
                        message.message = processNums(nums);
                        message.length = message.message.length() + sizeof(unsigned long);
                        std::cout << message << std::endl;
                    }
                    sendto(i, &message.length, sizeof(unsigned long), 0, (sockaddr *) &saUDP, sizeof saUDP);\
                    sendto(i, message.message.c_str(), message.length - sizeof(unsigned long), 0, (sockaddr *) &saUDP,
                           sizeof saUDP);
                } else {
                    std::cout << "Get message...";
                    fflush(stdout);
                    Message message = getMessage(i, &saTCP);
                    std::cout << message << " with length " << message.length << std::endl;
                    if (message.message == "exit") {
                        if (shutdown(i, SHUT_RDWR) == -1) {
                            perror("shutdown failed");
                            close(i);
                            close(socketTCP);
                            exit(EXIT_FAILURE);
                        }
                        close(i);
                        FD_CLR(i, &current_sockets);
                    } else {
                        std::vector<int> nums = getNums(message);
                        std::cout << nums.size() << std::endl;
                        if (!nums.empty()) {
                            message.message = processNums(nums);
                            message.length = message.message.length() + sizeof(unsigned long);
                            std::cout << message << std::endl;
                        }
                        sendMessage(i, message);
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}