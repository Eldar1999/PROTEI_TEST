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

struct Message {
    unsigned long length;
    std::string message;

    friend std::ostream &operator<<(std::ostream &os, Message &m);

    friend std::istream &operator>>(std::istream &is, Message &m);
};

std::ostream &operator<<(std::ostream &os, Message &m) {
    os << m.message;
    return os;
}

std::istream &operator>>(std::istream &is, Message &m) {
    is >> m.message;
    m.length = m.message.size() + sizeof(unsigned long);
    return is;
}

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

int raiseTCPServer(sockaddr_in &sa, int backLog) {
    std::cout << "TCP server startup" << std::endl;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check(s, "Socket error");

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

int raiseUDPServer(sockaddr_in &sa, int backLog) {
    std::cout << "UDP server startup" << std::endl;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    check(s, "Socket error");

    if (check(bind(s, (struct sockaddr *) &sa, sizeof sa), "Bind error") == -1) {
        close(s);
        return -1;
    }

    return s;
}

int raiseTCPClient(sockaddr_in &sa, int backLog) {
    std::cout << "TCP client startup" << std::endl;
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check(s, "Socket error");
    if (check(connect(s, (struct sockaddr *) &sa, sizeof sa), "connect failed") == -1) {
        close(s);
        return -1;
    }
    return s;
}

int raiseUDPClient(sockaddr_in &sa, int backLog) {
    std::cout << "UDP client startup" << std::endl;
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    check(s, "Socket error");
    if (check(connect(s, (struct sockaddr *) &sa, sizeof sa), "connect failed") == -1) {
        close(s);
        return -1;
    }
    return s;
}

int sendMessage(int fd, Message &msg) {
    ssize_t bytes_sent = sendto(fd, &msg.length, sizeof(unsigned long), MSG_NOSIGNAL, (struct sockaddr*) &sa, sizeof sa);
    while (bytes_sent < msg.length) {
        bytes_sent += send(fd, msg.message.c_str(), msg.length - sizeof(unsigned long), MSG_NOSIGNAL);
    }
    //perror("Error sending packet");
    //return EXIT_FAILURE;
    //retry_count = 0;
    // std::cout << bytes_sent << " bytes sent." << std::endl;

    return EXIT_SUCCESS;
}

Message getMessage(int fd, sockaddr_in *sa) {
    socklen_t s = sizeof(*sa);
    Message res{};
    recvfrom(fd, &res.length, sizeof(unsigned long), 0, (sockaddr *) sa, &s);
    ssize_t tmp = sizeof(unsigned long);
    res.message.resize(res.length >= tmp ? res.length - tmp : 0);

    while (tmp < res.length) {
        tmp += recvfrom(fd, &res.message[0], res.length - tmp, 0, (sockaddr *) sa, &s);
        if (tmp < 0) {
            perror("Read failure");
            exit(EXIT_FAILURE);
        }
    }
    //std::cout << res.length() << std::endl;
    return res;
}

std::vector<int> getNums(const Message &message) {
    std::vector<int> res;
    int sum = 0;
    int tmp = -1;
    for (char i: message.message) {
        if (i >= '0' && i <= '9') {
            if (tmp == -1) {
                tmp = 0;
            }
            tmp = tmp * 10 + int(i) - '0';
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
        tmp = -1;
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

void killApp(fd_set *ready_sockets) {

}

#endif //PROTEI_TEST_MYLIB_H
