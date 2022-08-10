#ifndef PROTEI_TEST_MYLIB_H
#define PROTEI_TEST_MYLIB_H

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include <bits/stdc++.h>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cerrno>
#include <poll.h>

typedef size_t msg_len_type;

struct message {
    msg_len_type *length = nullptr;
    uint8_t *msg;

    message() {
        auto tmp = calloc(1, 0xffff - 20);
        this->length = new(tmp) msg_len_type(0);
        this->msg = new((uint8_t *) tmp + sizeof(msg_len_type)) uint8_t[0xffff - 20 - sizeof(msg_len_type)]{};
    }

    ~message() {
        free(this->length);
    }

    friend std::ostream &operator<<(std::ostream &os, message &m);

    friend std::istream &operator>>(std::istream &is, message &m);

    message &operator=(const message &right) {

        if (this == &right) {
            return *this;
        }
        this->length = right.length;
        memcpy(this->msg, right.msg, 0xffff - 28);
        return *this;
    }

    char *operator+() const {
        return (char *) this->msg;
    }
};

std::ostream &operator<<(std::ostream &os, message &m) {
    os << (char *) m.msg;
    return os;
}

std::istream &operator>>(std::istream &is, message &m) {
    is >> (char *) m.msg;
    *m.length = strlen((char *) m.msg);
    return is;
}

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

namespace tcp {

    struct user {
    private:
        void _calc_read_free_space_and_payload() {
            if (this->read_pos <= this->r_append_pos) {
                if (this->read_pos == this->r_append_pos && this->to_handle) {
                    this->r_free_space = 0;
                } else {
                    this->r_free_space = this->buff_size - this->r_append_pos + this->read_pos;
                }
            } else {
                this->r_free_space = this->read_pos - this->r_append_pos;
            }
            this->r_payload = this->buff_size - this->r_free_space;
        }

        void _calc_send_free_space_and_payload() {
            if (this->send_pos <= this->s_append_pos) {
                if (this->send_pos == this->s_append_pos && this->to_send) {
                    this->s_free_space = 0;
                } else {
                    this->s_free_space = this->buff_size - this->s_append_pos + this->send_pos;
                }
            } else {
                this->s_free_space = this->send_pos - this->s_append_pos;
            }
            this->s_payload = this->buff_size - this->s_free_space;
        }

        int _read_buff(void *dest, msg_len_type count) {
            if (count < 1) {
                return 0;
            }
            if (count > r_payload) {
                return -1;
            }

            if (this->read_pos < this->r_append_pos) {
                memcpy(dest, this->recv_buff + this->read_pos, count);
                this->read_pos += count;
            } else {
                if (this->read_pos + count <= this->buff_size) {
                    memcpy(dest, this->recv_buff + this->read_pos, count);
                    this->read_pos += count;
                } else {
                    int64_t tmp = this->buff_size - this->read_pos;
                    memcpy(dest, this->recv_buff + this->read_pos, tmp);
                    memcpy((uint8_t *) dest + tmp, this->recv_buff, count - tmp);
                    this->read_pos = count - tmp;
                }
            }
            if (this->read_pos == this->r_append_pos) {
                this->to_handle = false;
            }
            this->_calc_read_free_space_and_payload();
            return 0;
        }

        int _place_buff(uint8_t *to_place, msg_len_type count) {
            if (count < 1) {
                return 0;
            }
            if (count > s_free_space) {
                return -1;
            }

            if (this->send_pos <= this->s_append_pos) {
                if (this->s_append_pos + count <= buff_size) {
                    memcpy(this->send_buff + this->s_append_pos, to_place, count);
                    this->s_append_pos += count;
                } else {
                    int64_t tmp = this->buff_size - s_append_pos;
                    memcpy(this->send_buff + this->s_append_pos, to_place, tmp);
                    memcpy(this->send_buff, to_place + tmp, count - tmp);
                    this->s_append_pos = count - tmp;
                }
            } else {
                memcpy(this->send_buff, to_place, count);
                this->s_append_pos += count;
            }
            this->to_send = true;
            this->_calc_send_free_space_and_payload();
            return 0;
        }

    public:
        int sock_fd;
        uint8_t *send_buff, *recv_buff;
        int64_t s_append_pos = 0, r_append_pos = 0;
        int64_t read_pos = 0, send_pos = 0;
        int64_t buff_size = 0x15;
        int64_t r_free_space = buff_size, s_free_space = buff_size;
        int64_t r_payload = 0, s_payload = 0;

        bool to_handle = false;
        bool to_send = false;
        bool to_close = false;

        explicit user(int sock_fd, int64_t buff_size = 0x15) {
            this->sock_fd = sock_fd;
            if (this->sock_fd == -1) {
                perror("User accepting error!");
                this->to_close = true;
            } else {
                this->buff_size = buff_size;
                this->send_buff = new uint8_t[this->buff_size];
                this->recv_buff = new uint8_t[this->buff_size];
            }
        }

        user(const user &other) {
            this->s_append_pos = other.s_append_pos;
            this->r_append_pos = other.r_append_pos;
            this->send_pos = other.send_pos;
            this->read_pos = other.read_pos;
            this->sock_fd = other.sock_fd;
            this->to_close = other.to_close;
            this->to_handle = other.to_handle;
            this->to_send = other.to_send;
            this->buff_size = other.buff_size;
            this->send_buff = new uint8_t[this->buff_size];
            this->recv_buff = new uint8_t[this->buff_size];
            std::copy(other.send_buff, other.send_buff + other.buff_size, this->send_buff);
            std::copy(other.recv_buff, other.recv_buff + other.buff_size, this->recv_buff);
        }

        ~user() {
            delete[] this->send_buff;
            delete[] this->recv_buff;
        }

        user &operator=(const user &other) {
            if (this == &other) {
                return *this;
            }
            if (this->buff_size != other.buff_size) {
                delete[] this->send_buff;
                delete[] this->recv_buff;
                this->buff_size = 0;
                this->send_buff = new uint8_t[other.buff_size];
                this->recv_buff = new uint8_t[other.buff_size];
                this->buff_size = other.buff_size;
            }
            this->s_append_pos = other.s_append_pos;
            this->r_append_pos = other.r_append_pos;
            this->send_pos = other.send_pos;
            this->read_pos = other.read_pos;
            this->sock_fd = other.sock_fd;
            this->to_close = other.to_close;
            this->to_send = other.to_send;
            this->to_handle = other.to_handle;
            std::copy(other.send_buff, other.send_buff + other.buff_size, this->send_buff);
            std::copy(other.recv_buff, other.recv_buff + other.buff_size, this->recv_buff);
            return *this;
        }

        user &operator=(user &&other) noexcept {
            if (this == &other) {
                return *this;
            }

            delete[] this->send_buff;
            delete[] this->recv_buff;

            this->s_append_pos = std::exchange(other.s_append_pos, 0);
            this->r_append_pos = std::exchange(other.r_append_pos, 0);
            this->send_pos = std::exchange(other.send_pos, 0);
            this->read_pos = std::exchange(other.read_pos, 0);
            this->sock_fd = std::exchange(other.sock_fd, 0);
            this->to_close = std::exchange(other.to_close, 0);
            this->to_send = std::exchange(other.to_send, 0);
            this->to_handle = std::exchange(other.to_handle, 0);
            this->send_buff = std::exchange(other.send_buff, nullptr);
            this->recv_buff = std::exchange(other.recv_buff, nullptr);

            return *this;
        }

        int try_to_read() {
            ssize_t res;
            if (r_free_space == 0) {
                return -1;
            }
            if (this->read_pos > this->r_append_pos) {
                res = recv(this->sock_fd, this->recv_buff + this->r_append_pos, this->read_pos - this->r_append_pos,
                           MSG_DONTWAIT);
                this->r_append_pos += res;
            } else if (this->read_pos <= this->r_append_pos) {
                int64_t tmp = this->buff_size - this->r_append_pos;
                res = recv(this->sock_fd, this->recv_buff + this->r_append_pos,
                           tmp,
                           MSG_DONTWAIT);
                if (res != -1) {
                    this->r_append_pos += res;
                    if (tmp < this->r_free_space && this->r_append_pos == this->buff_size - tmp) {
                        res = recv(this->sock_fd, this->recv_buff,
                                   this->read_pos,
                                   MSG_DONTWAIT);
                        if (res != -1) {
                            this->r_append_pos = res;
                        }
                    }
                } else {
                    this->r_append_pos += res;
                }
            }
            if (res == -1) {
                if (errno == EAGAIN) {
                    return 0;
                } else {
                    to_close = true;
                    return -1;
                }
            }

            if (res == 0) {
                to_close = true;
                return -1;
            }

            this->to_handle = true;
            this->_calc_read_free_space_and_payload();
            return 0;
        }

        int add_to_send(message &msg) {
            if (check(
                    this->_place_buff(reinterpret_cast<uint8_t *>(msg.length), sizeof(msg_len_type) + *msg.length),
                    "User " + std::to_string(this->sock_fd) + " don`t have free space in send buffer!") == -1) {
                return -1;
            }
            return 0;
        }

        int try_to_send() {
            ssize_t res;
            if (this->send_pos >= this->s_append_pos) {
                int64_t tmp = this->buff_size - this->send_pos;
                res = send(this->sock_fd, this->send_buff + this->send_pos, tmp,
                           MSG_DONTWAIT);
                if (res != -1) {
                    if (this->send_pos + res < this->buff_size) {
                        this->send_pos += res;
                    } else {
                        res = send(this->sock_fd, this->send_buff,
                                   this->s_append_pos,
                                   MSG_DONTWAIT);
                        this->send_pos = res;
                    }
                }
            } else if (this->send_pos < this->s_append_pos) {
                res = send(this->sock_fd, this->send_buff + this->send_pos, this->s_append_pos - this->send_pos,
                           MSG_DONTWAIT);
                this->send_pos += res;
            } else {
                return -1;
            }

            if (res == -1) {
                if (errno == EAGAIN) {
                    return 0;
                } else {
                    this->to_close = true;
                    return -1;
                }
            }
            if (this->send_pos == this->s_append_pos) this->to_send = false;
            this->_calc_send_free_space_and_payload();
            return 0;
        }

        int get_message(message &msg) {
            if (this->r_payload == 0) {
                this->to_handle = false;
                return 0;
            }
            memset(msg.msg, 0, *msg.length);
            *msg.length = 0;
            if (check(this->_read_buff(reinterpret_cast<uint8_t *>(msg.length), sizeof(msg_len_type)),
                      "User " + std::to_string(this->sock_fd) + " don`t have free space in send buffer!") == -1) {
                this->to_close = true;
                return -1;
            }
            if (check(this->_read_buff(msg.msg, *msg.length),
                      "User " + std::to_string(this->sock_fd) + " don`t have free space in send buffer!") == -1) {
                this->to_close = true;
                return -1;
            }
            return 0;
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
