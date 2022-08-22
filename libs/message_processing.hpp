//
// Created by abibulaev on 16.08.22.
//

#ifndef TEST_STRING_PROCESSING_H
#define TEST_STRING_PROCESSING_H

#include "message.hpp"
#include<vector>

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

#endif //TEST_STRING_PROCESSING_H
