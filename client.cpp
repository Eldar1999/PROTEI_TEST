#include "myLib.h"

#define TCP_PORT 1100
#define UDP_PORT 1101

int main(int argc, char **argv) {
    int tcp = (strcmp(argv[2], "TCP") == 0);
    if (argc != 3) {
        std::cout << "Usage: client <IPv4> <Protocol {TCP|UDP}" << std::endl;
        exit(1);
    }
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(tcp ? TCP_PORT : UDP_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (check(inet_pton(AF_INET, argv[1], &sa.sin_addr), "invalid IPv4") == -1) {
        return EXIT_FAILURE;
    }

    std::cout << "Get socket..." << (tcp ? TCP_PORT : UDP_PORT) << " ";
    int s = (tcp ? raiseTCPClient(sa, 1) : raiseUDPClient(sa, 1));
    if (check(s, "Socket error") == -1) {
        exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    while (true) {
        std::string message;
        std::string answer;
        std::cout << "Enter your message or write 'exit' to exit: ";
        fflush(stdout);
        getline(std::cin, message);
        send(s, (void *) message.c_str(), message.length() + 2);
        if (message == "exit") {
            break;
        }
        answer = getMessage(s, &sa);
        std::cout << "Answer: " << answer.c_str() << std::endl;
    }
    std::cout << "Socket closing...";
    if (check(close(s), "Socket closing error!") == -1) {
        return EXIT_FAILURE;
    }
    std::cout << "OK" << std::endl;
    fflush(stdout);

    return EXIT_SUCCESS;
}