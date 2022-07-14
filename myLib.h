#ifndef PROTEI_TEST_MYLIB_H
#define PROTEI_TEST_MYLIB_H

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <bits/stdc++.h>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <cerrno>

typedef unsigned short msg_len_type;

struct Message {
    char *message;

    Message() {
        this->message = new char[0xffff];
    }

    ~Message() {
        delete[] this->message;
    }

    friend std::ostream &operator<<(std::ostream &os, Message &m);

//    friend std::istream &operator>>(std::istream &is, Message &m);

    Message &operator=(const Message &right) {

        if (this == &right) {
            return *this;
        }
        memcpy(this->message, right.message, 0xffff);
        return *this;
    }
};

std::ostream &operator<<(std::ostream &os, Message &m) {
    os << (char *) (m.message + sizeof(msg_len_type));
    return os;
}

//std::istream &operator>>(std::istream &is, Message &m) {
//    is >> m.message;
//    m.length = strlen(m.message) + sizeof(unsigned long);
//    return is;
//}

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

namespace tcp {

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

    int send_message(int fd, Message &msg) {
        ssize_t bytes_sent = 0;
        while (bytes_sent < *(msg_len_type *) msg.message) {
            bytes_sent += check((int) send(fd, msg.message, *(msg_len_type *) msg.message, MSG_NOSIGNAL),
                                "Sending error!");
        }
        //perror("Error sending packet");
        //return EXIT_FAILURE;
        //retry_count = 0;
        // std::cout << bytes_sent << " bytes sent." << std::endl;

        return EXIT_SUCCESS;
    }

    Message get_message(int fd, sockaddr_in *sa) {
        socklen_t s = sizeof(*sa);
        Message res{};
        ssize_t bytes_received = check((int) recv(fd, res.message, sizeof(msg_len_type), 0), "Receive error: ");

        while (bytes_received < *(msg_len_type *) res.message) {
            bytes_received += recvfrom(fd, res.message + sizeof(msg_len_type), *(msg_len_type *) res.message - 2, 0,
                                       (sockaddr *) sa, &s);
            if ((int) bytes_received < 0) {
                perror("Read failure");
                exit(EXIT_FAILURE);
            }
        }
        //std::cout << res.length() << std::endl;
        return res;
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

    int send_message(int fd, Message &msg, sockaddr_in *sa) {
        socklen_t s = sizeof(*sa);
        ssize_t bytes_sent = check((int) sendto(fd, msg.message, sizeof(msg_len_type), 0,(sockaddr *) sa,  s),
                                   "Sending error!");

        while (bytes_sent < *(msg_len_type *) msg.message) {
            bytes_sent += check((int) sendto(fd, msg.message, *(msg_len_type *) msg.message, 0,(sockaddr *) sa,  s),
                                "Sending error!");
        }
        //perror("Error sending packet");
        //return EXIT_FAILURE;
        //retry_count = 0;
        // std::cout << bytes_sent << " bytes sent." << std::endl;

        return EXIT_SUCCESS;
    }

    Message get_message(int fd, sockaddr_in *sa) {
        socklen_t s = sizeof(*sa);
        Message res{};
        ssize_t bytes_received = check((int) recvfrom(fd, res.message, sizeof(msg_len_type), MSG_PEEK, (sockaddr *) sa, &s),
                                       "Receive error: ");

        bytes_received = 0;

        while (bytes_received < *(msg_len_type *) res.message) {
            bytes_received += recvfrom(fd, res.message, *(msg_len_type *) res.message, 0,
                                       (sockaddr *) sa, &s);
            if ((int) bytes_received < 0) {
                perror("Read failure");
                exit(EXIT_FAILURE);
            }
        }
        return res;
    }
}


std::vector<int> getNums(const Message &message) {
    std::vector<int> res;
    int sum = 0;
    int tmp = -1;
    for (int i = sizeof(msg_len_type); i < *(msg_len_type *) message.message; i++) {
        if (message.message[i] >= '0' && message.message[i] <= '9') {
            if (tmp == -1) {
                tmp = 0;
            }
            tmp = tmp * 10 + int(message.message[i]) - '0';
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

std::string processNums(std::vector<int> vec) {
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

//void killApp(fd_set *ready_sockets) {
//
//}

#endif //PROTEI_TEST_MYLIB_H
