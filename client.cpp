#include "client_server.h"

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

    struct pollfd fd = {s, POLL_OUT | POLL_IN, 0};

    tcp::user user(s);

    while (true) {
        message msg{};
        message answ{};
        std::cout << "Enter your msg or write 'exit' to exit: ";
        fflush(stdout);
        fgets((char *) msg.msg, 0xffff - sizeof(msg_len_type), stdin);
        msg.msg[strlen((char *) msg.msg) - 1] = '\0';
        *msg.length = strlen((char *) msg.msg);
        user.add_to_send(msg);
        user.try_to_send();
        if (!strcmp((char *) msg.msg, "exit")) {
            break;
        }
        if (poll(&fd, 1, 100)) {
            if (fd.revents & POLLIN) {
                if (user.try_to_read() == -1) {
                    perror("Message reading error!");
                    close(s);
                    exit(EXIT_FAILURE);
                }
                while (user.to_handle && user.get_message(answ) != -1)
                    std::cout << "Answer: " << answ << std::endl;
            }
        }

    }
    std::cout << "Socket closing...";
    if (check(close(s), "Socket closing error!") == -1) {
        return EXIT_FAILURE;
    }
    std::cout << "OK" << std::endl;
    fflush(stdout);

    return EXIT_SUCCESS;
}