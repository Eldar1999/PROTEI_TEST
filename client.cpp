#include "myLib.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: client <IPv4> <Protocol {TCP|UDP}" << std::endl;
        exit(1);
    }
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (check(inet_pton(AF_INET, argv[1], &sa.sin_addr), "invalid IPv4") == -1) {
        return EXIT_FAILURE;
    }

    std::cout << "Get socket...";
    int s = raise_client(sa, 1);
    check(s, "Socket error");
    std::cout << "OK" << std::endl;

    while (true) {
        std::string message;
        char answer[1024] = {0};
        getline(std::cin, message);
        send(s, (void *) message.c_str(), message.length() + 2);
        recv(s, answer, 1024, 0);
        std::cout << answer << std::endl;
        break;

    }
    /* perform read write operations ... */

    close(s);
    return EXIT_SUCCESS;
}