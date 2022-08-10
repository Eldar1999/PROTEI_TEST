//
// Created by abibulaev on 10.08.22.
//

#ifndef MY_PROJECT_CIRCLE_QUEUE_H
#define MY_PROJECT_CIRCLE_QUEUE_H

#include <cstdint>
#include <cstring>

struct circle_queue {
    uint8_t *buff;
    uint64_t buff_size = 0x10;
    uint64_t append_pos = 0, read_pos = 0;
    uint64_t free_space = this->buff_size;
    uint64_t payload = 0;
    bool to_read = false;

    circle_queue() {
        this->buff = new uint8_t[this->buff_size]{};
    }

    circle_queue(uint64_t size) {
        this->buff_size = size;
        this->buff = new uint8_t[this->buff_size]{};
        this->free_space = this->buff_size;
    }

    ~circle_queue() {
        delete[] this->buff;
    }

    int push(uint64_t len, uint8_t *data) {
        if (len > this->free_space) {
            return -1;
        } else {
            if (this->append_pos + len > this->buff_size) {
                uint64_t tmp = this->buff_size - this->append_pos;
                memcpy(this->buff + this->append_pos, data, tmp);
                memcpy(this->buff, data + tmp, len - tmp);
                this->append_pos = len - tmp;
            } else {
                memcpy(this->buff + this->append_pos, data, len);
                this->append_pos += len;
            }
            this->payload += len;
            this->free_space -= len;
            this->to_read = true;
        }
        return 0;
    }

    int pop(uint64_t len, uint8_t *buffer) {
        if (len > this->payload) {
            return -1;
        } else {
            if (this->read_pos + len > this->buff_size) {
                uint64_t tmp = this->buff_size - this->read_pos;
                memcpy(buffer, this->buff + this->read_pos, tmp);
                memcpy(buffer + tmp, this->buff, len - tmp);
                this->read_pos = len - tmp;
            } else {
                memcpy(buffer, this->buff + this->read_pos, len);
                this->read_pos += len;
            }
            this->payload -= len;
            this->free_space += len;
            this->to_read = (this->payload > 0);
        }
        return 0;
    }

};


#endif //MY_PROJECT_CIRCLE_QUEUE_H
