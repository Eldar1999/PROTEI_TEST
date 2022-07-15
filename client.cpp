#include "myLib.h"

#define TCP_PORT 1100
#define UDP_PORT 1101


int main(int argc, char **argv) {
    int flag = (strcmp(argv[2], "TCP") == 0);
    if (argc != 3) {
        std::cout << "Usage: client <IPv4> <Protocol {TCP|UDP}" << std::endl;
        exit(1);
    }
    sockaddr_in sa{};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(flag ? TCP_PORT : UDP_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (check(inet_pton(AF_INET, argv[1], &sa.sin_addr), "invalid IPv4") == -1) {
        return EXIT_FAILURE;
    }

    std::cout << "Get socket..." << (flag ? TCP_PORT : UDP_PORT) << " ";
    int s = (flag ? tcp::raise_client(sa) : udp::raise_client(sa));
    if (check(s, "Socket error") == -1) {
        exit(EXIT_FAILURE);
    }
    std::cout << "OK" << std::endl;

    while (true) {
        Message message;
        Message answer;
        std::cout << "Enter your message or write 'exit' to exit: ";
        fflush(stdout);
        fgets(message.message + sizeof(msg_len_type), 0xffff - sizeof(msg_len_type), stdin);
        message.message[strlen(message.message + sizeof(msg_len_type)) + sizeof(msg_len_type) - 1] = '\0';
        *(msg_len_type *) message.message =
                strlen(message.message + sizeof(msg_len_type)) + sizeof(msg_len_type) + 1;
        flag ? tcp::send_message(s, message) : udp::send_message(s, message, &sa);
        if (!strcmp(message.message + 2, "exit")) {
            break;
        }
        do {
            answer = flag ? tcp::get_message(s, &sa) : udp::get_message(s, &sa);
        } while (get_msg_len(answer.message) == 0);
        std::cout << "Answer: " << answer << std::endl;
    }
    std::cout << "Socket closing...";
    if (check(close(s), "Socket closing error!") == -1) {
        return EXIT_FAILURE;
    }
    std::cout << "OK" << std::endl;
    fflush(stdout);

    return EXIT_SUCCESS;
}