#include "myLib.h"

int main() {
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
                    std::string message = getMessage(i, &saUDP);
                    fflush(stdin);
                    std::cout << message.c_str() << std::endl;
                    std::vector<int> nums = getNums(message);
                    std::cout << nums.size() << std::endl;
                    if (!nums.empty()) {
                        message = processNums(nums);
                        std::cout << message << std::endl;
                    }
                    sendto(i, &message[0], message.length() + 2, 0, (sockaddr *) &saUDP, sizeof saUDP);
                } else {
                    std::cout << "Get message...";
                    fflush(stdout);
                    std::string message = getMessage(i, &saTCP);
                    std::cout << message.c_str() << std::endl;
                    if (strcmp(message.c_str(), "exit") == 0) {
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
                            message = processNums(nums);
                            std::cout << message << std::endl;
                        }
                        send(i, &message[0], message.length() + 2);
                    }
                }
            }
        }
    }

    close(socketTCP);
    return EXIT_SUCCESS;
}