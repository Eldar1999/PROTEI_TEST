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

int check(int exp, const std::string &msg) {
    if (exp == -1) {
        perror(msg.c_str());
    }
    return exp;
}

int raise_server(sockaddr_in &sa, int backLog) {
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

int raise_client(sockaddr_in &sa, int backLog) {
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    check(s, "Socket error");
    if (check(connect(s, (struct sockaddr *) &sa, sizeof sa), "connect failed") == -1) {
        close(s);
        return -1;
    }
    return s;
}

int acceptConnection(int sock) {
    return 0;
}

int send(int fd, void *buff, int N) {
    bool retry = true;
    int retry_count = 0;

    while (retry) {
        retry = false;
        ssize_t bytes_sent = send(fd, buff, N, MSG_NOSIGNAL);
        /**/
        if ((bytes_sent < 0 || bytes_sent < N) && retry_count <= 1000) {
            retry_count += 1;
            retry = true;
        } else if (bytes_sent < 0 && retry_count > 1000) {
            perror("Error sending packet");
            return EXIT_FAILURE;
        } else {
            retry_count = 0;
            std::cout << bytes_sent << " bytes sent." << std::endl;
        }
    }
    return EXIT_SUCCESS;
}

std::string getMessage(int fd) {
    std::string res;
    ssize_t offset = 0;
    res.resize(1024);
    bool tryToRead = true;
    int tmp = 0;
    while (tryToRead) {
        tmp = recv(fd, &res[offset], 1024, 0);
        if (tmp < 0) {
            perror("Read failure");
            exit(EXIT_FAILURE);
        } else if (tmp == 0 || res[tmp] == 0) {
            tryToRead = false;
        } else {
            res.resize(res.size() + 1024);
            offset += 1024;
        }
    }
    res.resize(tmp);
    return res;
}

std::vector<int> getNums(const std::string &str) {
    std::vector<int> res;
    int sum = 0;
    int tmp = -1;
    for (char i: str) {
        if (i >= '0' && i <= '9') {
            if (tmp == -1) {
                tmp = 0;
            }
            tmp = tmp * 10 + int(i) - '0';
            std::cout << i << " = " << tmp << std::endl;
        } else {
            if (tmp != -1) {
                res.push_back(tmp);
                sum += tmp;
                tmp = -1;
            }
        }
    }
    std::sort(res.begin(), res.end());
    if (!res.empty()) {
        res.push_back(sum);
    }
    return res;
}

std::string processNums(std::vector<int> vec) {
    std::string res = "";
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
