#include "client_server.hpp"

#define MAX_CONNECTIONS 200

int main() {
    struct pollfd fds[MAX_CONNECTIONS]{};
    for (int i = 3; i < MAX_CONNECTIONS; ++i) {
        fds[i].fd = -1;
    }

    timeval t{};
    t.tv_sec = 0;
    t.tv_usec = 1000;

    sockaddr_in saTCP{}, saUDP{};
    saTCP.sin_family = AF_INET;
    saTCP.sin_port = htons(1100);
    saTCP.sin_addr.s_addr = htonl(INADDR_ANY);

    saUDP.sin_family = AF_INET;
    saUDP.sin_port = htons(1101);
    saUDP.sin_addr.s_addr = htonl(INADDR_ANY);

    int socketTCP = tcp::raise_server(saTCP, 1);
    if (check(socketTCP, "Unable to get TCP socket") == -1) {
        return 1;
    }
    int socketUDP = udp::raise_server(saUDP);
    if (check(socketUDP, "Unable to get UDP socket") == -1) {
        return 1;
    }
    fds[socketTCP].fd = socketTCP;
    fds[socketTCP].events = POLLIN;
    fds[socketUDP].fd = socketUDP;
    fds[socketUDP].events = POLLIN;

    std::unordered_map<int, tcp::tcp_user> tcp_users;
    udp::udp_users_handler u(socketUDP);

    for (;;) {
        usleep(1);
        int fds_count = poll(fds, MAX_CONNECTIONS, 1000);
        if (fds_count == 0) {
            continue;
        } else if (fds_count == -1) {
            check(-1, "Poll error!");
            return EXIT_FAILURE;
        } else {
            for (int i = 3; i < MAX_CONNECTIONS; i++) {
                if (fds[i].revents & (POLLIN | POLLOUT)) {
                    if (i == socketUDP) {
                        std::cerr << "UDP datagrams received. Try to handle..." << "\n";
                        std::pair<sockaddr_in, message> msg{};
                        while (not u.try_to_read(msg)) {
                            if (u.to_handle) {
                                std::cerr << "\nNew message from: " << inet_ntoa(*reinterpret_cast<in_addr *>(&msg.first))
                                          << "\n";
                                std::cerr << "Msg text is: \"" << msg.second << "\"\nNums...";
                                auto tmp = process_nums(get_nums(msg.second));
                                if (tmp.length() > 0) {
                                    std::cerr << "detected!\nNew answer is: ";
                                    strcpy((char *) msg.second.msg, tmp.c_str());
                                    *msg.second.length = strlen((char *) msg.second.msg);
                                    std::cerr << msg.second << "\n";
                                } else {
                                    std::cerr << "note detected\nReply message\n";
                                };
                                std::cerr << "Adding msg to send...";
                                u.add_to_send(msg);
                                std::cerr << "OK\nSending...";
                                u.try_to_send();
                                std::cerr << "DONE!\n\n";
                            } else {
                                std::cerr << "Handling completed" << std::endl;
                                break;
                            }
                        }
                        if (u.to_close) {
                            perror("Socket error!");
                            close(socketUDP);
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (i == socketTCP) {
                        int s = accept(i, nullptr, nullptr);
                        if (s == -1) {
                            perror("Connection accept ERROR!");
                            continue;
                        }
                        tcp_users.emplace(s, tcp::tcp_user{s});
                        auto u = &tcp_users.at(s);
                        fds[u->sock_fd].fd = u->sock_fd;
                        fds[u->sock_fd].events = POLLIN;
                        perror(("New User " + std::to_string(u->sock_fd) + " connection...").c_str());
                        errno = 0;
                    } else {
                        if (tcp_users.contains(i)) {
                            auto u = &tcp_users.at(i);
                            if ((fds[i].events & POLLIN) && u->try_to_read() == -1) {
                                if (u->to_close) {
                                    perror(("Reading error acquired with user " + std::to_string(u->sock_fd)).c_str());
                                    errno = 0;
                                    std::cerr << "Closing...";
                                    fds[u->sock_fd].fd = -1;
                                    fds[u->sock_fd].events = 0;
                                    tcp_users.erase(u->sock_fd);
                                    std::cerr << "Done!" << std::endl;
                                } else if (u->rec_buf->free_space == 0 || u->snd_buf->free_space == 0) {
                                    fds[i].events ^= POLLIN;
                                }
                            }
                            message msg;
                            while (u->to_handle) {
                                u->get_message(msg);
                                if (!strcmp((char *) msg.msg, "exit")) {
                                    std::cerr << ("User " + std::to_string(u->sock_fd) + " want do disconnect!").c_str()
                                              << std::endl;
                                    std::cerr << "Disconnecting user...";
                                    fds[u->sock_fd].fd = -1;
                                    fds[u->sock_fd].events = 0;
                                    tcp_users.erase(u->sock_fd);
                                    std::cerr << "Done!" << std::endl;
                                    break;
                                }
                                auto tmp = process_nums(get_nums(msg));
                                if (tmp.length() > 0) {
                                    strcpy((char *) msg.msg, tmp.c_str());
                                    *msg.length = strlen((char *) msg.msg);
                                }
                                if (u->add_to_send(msg) == -1) {
                                    std::cerr << "Sending queue ended" << std::endl;
                                    fds[u->sock_fd].fd = -1;
                                    fds[u->sock_fd].events = 0;
                                    tcp_users.erase(u->sock_fd);
                                    break;
                                }
                                if (u->try_to_send() == -1) {
                                    if (u->to_close) {
                                        perror(("User " + std::to_string(u->sock_fd) +
                                                ": sending fatal error").c_str());
                                        fds[u->sock_fd].fd = -1;
                                        fds[u->sock_fd].events = 0;
                                        tcp_users.erase(u->sock_fd);
                                    } else {
                                        fds[u->sock_fd].events |= POLLOUT;
                                    }
                                }
                                if (u->rec_buf->free_space > 0 && u->snd_buf->free_space > 0) {
                                    fds[i].events |= POLLIN;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}