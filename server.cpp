#include "myLib.h"

int main() {
    timeval t{};
    t.tv_sec = 1;
    t.tv_usec = 0;

    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);


    int s = raise_server(sa, 1);

    if (s == -1) {
        return 1;
    }

    fd_set current_sockets, ready_sockets, write_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(s, &current_sockets);

    for (;;) {
        sleep(1);
        //std::cout<<".";
        ready_sockets = current_sockets;
        write_sockets = current_sockets;

        check(select(FD_SETSIZE, &ready_sockets, &write_sockets, nullptr, &t), "select error");
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == s) {
                    int ConnectFD = accept(s, nullptr, nullptr);

                    if (check(ConnectFD, "accept failed") == -1) {
                        close(s);
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(ConnectFD, &ready_sockets);
                } else {
                    std::cout << "Get message...";
                    fflush(stdout);
                    std::string message = getMessage(i);
                    std::cout << message << std::endl;
                    std::vector<int> nums = getNums(message);
                    std::cout << nums.size() << std::endl;
                    if (!nums.empty()) {
                        message = processNums(nums);
                        std::cout << message << std::endl;
                    }
                    send(i, &message[0], message.length() + 2);


                    /* perform read write operations ...*/
                    /**/

                    if (shutdown(i, SHUT_RDWR) == -1) {
                        perror("shutdown failed");
                        close(i);
                        close(s);
                        exit(EXIT_FAILURE);
                    }
                    close(i);
                    FD_CLR(i, &ready_sockets);
                }


            }
        }
    }

    close(s);
    return EXIT_SUCCESS;
}