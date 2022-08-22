#ifndef PROTEI_TEST_MYLIB_H
#define PROTEI_TEST_MYLIB_H

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unordered_map>
#include <bits/stdc++.h>
#include <csignal>
#include <unistd.h>
#include <cerrno>
#include <poll.h>
#include "message.hpp"
#include "circle_queue.hpp"
#include "message_processing.hpp"

#define BUFF_SIZE 0xffff

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

thread_local uint8_t temp_buff[0xffff];

namespace tcp {

    struct tcp_user {
    public:
        int sock_fd;
        circle_queue *rec_buf = nullptr, *snd_buf = nullptr;

        bool to_handle = false;
        bool to_send = false;
        bool to_close = false;

        explicit tcp_user(int sock_fd, int64_t buff_size = BUFF_SIZE) {
            this->sock_fd = sock_fd;
            if (this->sock_fd == -1) {
                perror("User accepting error!");
                this->to_close = true;
            } else {
                this->rec_buf = new circle_queue(buff_size);
                this->snd_buf = new circle_queue(buff_size);
            }
        }

        tcp_user(tcp_user &&other) noexcept {
            this->sock_fd = std::exchange(other.sock_fd, -1);
            this->rec_buf = std::exchange(other.rec_buf, nullptr);
            this->snd_buf = std::exchange(other.snd_buf, nullptr);
            this->to_close = std::exchange(other.to_close, false);
            this->to_send = std::exchange(other.to_send, false);
            this->to_handle = std::exchange(other.to_handle, false);
        }

        ~tcp_user() {
            delete this->snd_buf;
            delete this->rec_buf;
        }

        int try_to_read() {
            if (this->rec_buf->free_space == 0) {
                return -1;
            } else {
                ssize_t res;
                res = recv(this->sock_fd, temp_buff, rec_buf->free_space, MSG_DONTWAIT);
                if (res == -1) {
                    if (errno == EAGAIN) {
                        return 0;
                    } else {
                        this->to_close = true;
                        return -1;
                    }
                }
                if (res == 0) {
                    exit(EXIT_FAILURE);
                }
                this->rec_buf->push(res, temp_buff);
                this->to_handle = true;
            }
            return 0;
        }

        int add_to_send(message &msg) {
            if (this->snd_buf->free_space == 0) {
                return -1;
            } else {
                this->snd_buf->push(*msg.length + sizeof(msg_len_type) + sizeof(user_id_type) + sizeof(flag_type),
                                    reinterpret_cast<uint8_t *>(msg.length));
                this->to_send = true;
                return 0;
            }
        }

        int try_to_send() {
            size_t bytes_to_send = this->snd_buf->payload;
            this->snd_buf->pop(this->snd_buf->payload, temp_buff);
            ssize_t res = send(this->sock_fd, temp_buff, bytes_to_send, MSG_DONTWAIT);
            if (res == -1) {
                if (errno == EAGAIN) {
                    return 0;
                } else if (errno == ENOBUFS) {
                    return 1;
                } else {
                    return -1;
                }
            }
            this->to_send = false;
            return 0;
        }

        int get_message(message &msg) {
            if (this->rec_buf->payload == 0) {
                return -1;
            } else {
                this->rec_buf->pop(sizeof(msg_len_type), reinterpret_cast<uint8_t *>(msg.buffer));
                this->rec_buf->pop(*msg.length + sizeof(user_id_type) + sizeof(flag_type),
                                   reinterpret_cast<uint8_t *>(msg.buffer + sizeof(msg_len_type)));
                if (this->rec_buf->payload == 0) {
                    this->to_handle = false;
                }
            }
            return 0;
        }
    };

    int raise_server(sockaddr_in &sa, int backLog) {
        std::cout << "TCP server startup" << std::endl;
        int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        check(s, "Socket error");

        if (check(bind(s, (struct sockaddr *) &sa, sizeof sa), "Bind error") == -1) {
            exit(EXIT_FAILURE);
        }

        if (check(listen(s, backLog), "listen error") == -1) {
            exit(EXIT_FAILURE);
        }
        return s;
    }

    int raise_client(sockaddr_in &sa) {
        std::cout << "TCP client startup" << std::endl;
        int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        check(s, "Socket error");
        if (check(connect(s, (struct sockaddr *) &sa, sizeof sa), "connect failed") == -1) {
            close(s);
            return -1;
        }
        return s;
    }
}

namespace udp {
    struct udp_users_handler {
    public:
        int sock_fd;
        std::queue<std::pair<sockaddr_in, message>> buf{};

        bool to_handle = false;
        bool to_send = false;
        bool to_close = false;

        explicit udp_users_handler(int sock_fd) {
            this->sock_fd = sock_fd;
            if (this->sock_fd == -1) {
                perror("User accepting error!");
                this->to_close = true;
            }
        }

        udp_users_handler(udp_users_handler &&other) noexcept {
            this->sock_fd = std::exchange(other.sock_fd, -1);
            this->buf = std::exchange(other.buf, this->buf);
            this->to_close = std::exchange(other.to_close, false);
            this->to_send = std::exchange(other.to_send, false);
            this->to_handle = std::exchange(other.to_handle, false);
        }

        int try_to_read(std::pair<sockaddr_in, message> &msg) {
            this->to_handle = false;
            socklen_t in_addr_len = sizeof(sockaddr_in);
            ssize_t res;

            res = recvfrom(this->sock_fd, msg.second.buffer, 0xffff - 20, MSG_DONTWAIT,
                           reinterpret_cast<sockaddr *>(&msg.first), &in_addr_len);
            if (res == -1) {
                if (errno == EAGAIN) {
                    return 0;
                } else {
                    this->to_close = true;
                    return -1;
                }
            }
            if (res == 0) {
                exit(EXIT_FAILURE);
            }
            this->to_handle = true;
            return 0;
        }

        int try_to_send() {
            ssize_t res = 1;
            while (res > 0 && not this->buf.empty()) {
                res = sendto(this->sock_fd, this->buf.front().second.buffer,
                             this->buf.front().second.get_msg_size(), MSG_DONTWAIT,
                             reinterpret_cast<const sockaddr *>(&this->buf.front().first),
                             sizeof(sockaddr_in));
                this->buf.pop();
            }

            if (res == -1) {
                if (errno == EAGAIN) {
                    return 0;
                } else if (errno == ENOBUFS) {
                    return 1;
                } else {
                    return -1;
                }
            }
            this->to_send = false;
            return 0;
        }

        int add_to_send(std::pair<sockaddr_in, message> &msg) {
            this->buf.push(msg);
            this->to_send = true;
            return 0;
        }
    };


    int raise_server(sockaddr_in &sa) {
        std::cout << "UDP server startup" << std::endl;
        int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        check(s, "Socket error");

        if (check(bind(s, (struct sockaddr *) &sa, sizeof sa), "Bind error") == -1) {
            close(s);
            return -1;
        }
        return s;
    }

    int raise_client(sockaddr_in &sa) {
        std::cout << "UDP client startup" << std::endl;
        int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        check(s, "Socket error");
        return s;
    }


}

#endif //PROTEI_TEST_MYLIB_H
