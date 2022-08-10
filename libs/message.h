//
// Created by abibulaev on 10.08.22.
//

#ifndef MY_PROJECT_MESSAGE_H
#define MY_PROJECT_MESSAGE_H

#include<iostream>
#include<cstring>

typedef size_t msg_len_type;

struct message {
    msg_len_type *length = nullptr;
    uint8_t *msg;

    message() {
        auto tmp = new uint8_t[0xffff - 20];
        this->length = new(tmp) msg_len_type(0);
        this->msg = new((uint8_t *) tmp + sizeof(msg_len_type)) uint8_t[0xffff - 20 - sizeof(msg_len_type)]{};
    }

    message(message &right){
        auto tmp = new uint8_t[0xffff - 20];
        this->length = new(tmp) msg_len_type(*right.length);
        this->msg = new(tmp + sizeof(msg_len_type)) uint8_t[0xffff - 20 - sizeof(msg_len_type)]{};
        memcpy(this->msg, right.msg, 0xffff - 28);
    }

    message(msg_len_type len, char *str) {
        auto tmp = new uint8_t[0xffff - 20];
        this->length = new(tmp) msg_len_type(len);
        this->msg = new(tmp + sizeof(msg_len_type)) uint8_t[0xffff - 20 - sizeof(msg_len_type)]{};
        memcpy(this->msg, str, len);
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
        *this->length = *right.length;
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


#endif //MY_PROJECT_MESSAGE_H
