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
#include "message.h"
#include "circle_queue.h"

#define BUFF_SIZE 0x15

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

thread_local uint8_t temp_buff[0xffff];

namespace tcp {

    struct user {
    public:
        int sock_fd;
        circle_queue *rec_buf, *snd_buf;

        bool to_handle = false;
        bool to_send = false;
        bool to_close = false;

        explicit user(int sock_fd, int64_t buff_size = BUFF_SIZE) {
            this->sock_fd = sock_fd;
            if (this->sock_fd == -1) {
                perror("User accepting error!");
                this->to_close = true;
            } else {
                this->rec_buf = new circle_queue(buff_size);
                this->snd_buf = new circle_queue(buff_size);
            }
        }

        ~user() {
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
        }

        int add_to_send(message &msg) {
            if (this->snd_buf->free_space == 0) {
                return -1;
            } else {
                this->snd_buf->push(*msg.length, reinterpret_cast<uint8_t *>(msg.length));
                this->to_send = true;
                return 0;
            }
        }

        int try_to_send() {
            ssize_t res = this->snd_buf->payload;
            this->snd_buf->pop(this->snd_buf->payload, temp_buff);
            res = send(this->sock_fd, temp_buff, res, MSG_DONTWAIT);
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
        }

        int get_message(message &msg) {
            if (this->rec_buf->payload == 0){
                return -1;
            } else {
                this->rec_buf->pop(sizeof(msg_len_type), reinterpret_cast<uint8_t *>(msg.length));
                this->rec_buf->pop(*msg.length, msg.msg);
                if (this->rec_buf->payload == 0){
                    this->to_handle = false;
                }
            }
        }

    };

    int raise_server(sockaddr_in &sa, int backLog) {
        std::cout << "TCP server startup" << std::endl;
        int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        check(s, "Socket error");
        //setsockopt(s, SOL_SOCKET, SO_REUSEADDR, nullptr, 0);

        if (check(bind(s, (struct sockaddr *) &sa, sizeof sa), "Bind error") == -1) {
            close(s);
            return -1;
        }

        if (check(listen(s, backLog), "listen error") == -1) {
            close(s);
            return -1;
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


std::vector<int> get_nums(const message &message) {
    std::vector<int> res;
    int sum = 0;
    int tmp = -1;
    for (msg_len_type i = 0; i < *message.length; i++) {
        if (message.msg[i] >= '0' && message.msg[i] <= '9') {
            if (tmp == -1) {
                tmp = 0;
            }
            tmp = tmp * 10 + int(message.msg[i]) - '0';
            // std::cout << i << " = " << tmp << std::endl;
        } else {
            if (tmp != -1) {
                res.push_back(tmp);
                sum += tmp;
                tmp = -1;
            }
        }
    }
    if (tmp != -1) {
        res.push_back(tmp);
        sum += tmp;
    }
    std::sort(res.begin(), res.end());
    if (!res.empty()) {
        res.push_back(sum);
    }
    return res;
}

std::string process_nums(std::vector<int> vec) {
    std::string res;
    if (vec.empty()) {
        return "";
    } else {
        for (int i = 0; i < vec.size() - 2; i++) {
            res += std::to_string(vec[i]) + " ";
        }
        res += std::to_string(vec[vec.size() - 2]) + "\n" + std::to_string(vec[vec.size() - 1]);
    }
    return res;
}

#endif //PROTEI_TEST_MYLIB_H
