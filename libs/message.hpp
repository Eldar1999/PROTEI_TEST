//
// Created by abibulaev on 10.08.22.
//

#ifndef MY_PROJECT_MESSAGE_H
#define MY_PROJECT_MESSAGE_H

#include<iostream>
#include<cstring>

typedef size_t msg_len_type;
typedef short user_id_type;
typedef unsigned char flag_type;


/** Bitset for flags.\n
 *
 *  @param 0 bit -> auth (not implemented)\n TODO
 *  @param 1 bit -> accept (not implemented)\n TODO
 *  @param 2 bit -> none\n
 *  @param 3 bit -> none\n
 *  @param 4 bit -> none\n
 *  @param 5 bit -> none\n
 *  @param 6 bit -> none\n
 *  @param 7 bit -> none\n
**/
struct flags {
    /**
     * Get bit value
     * @param bit_num number of bit to get value
     * @return \<<b>bit_num</b>\> bit in position  value
     */
    bool get_bit(const int bit_num) const {
        return this->f & 1 << bit_num;
    }

    /**
     * Get bit value
     * @param bit_num number of bit to get value
     * @return \<<b>bit_num</b>\> bit in position  value
     */
    bool operator[](const int bit_num) const {
        return this->f & 1 << bit_num;
    }

    /**
     * Invert bit value
     * @param bit_num number of bit to invert
     */
    void change_bit(int bit_num) {
        this->f ^= 1 << bit_num;
    }
private:
    flag_type f;
};

/**
 * Message structure for server-client conversation
 */
struct message {
public:
    uint8_t *buffer;
    msg_len_type *length = nullptr;
    user_id_type *id = nullptr;
    flag_type *flags = nullptr;
    uint8_t *msg = nullptr;

    message() {
        this->buffer = new uint8_t[0xffff - 20];
        int offset = 0;
        this->length = new(this->buffer) msg_len_type(0);
        offset += sizeof(msg_len_type);
        this->id = new(this->buffer + offset) user_id_type(0);
        offset += sizeof(user_id_type);
        this->flags = new(this->buffer + offset) flag_type(0);
        offset += sizeof(flag_type);
        this->msg = new(this->buffer + offset) uint8_t[0xffff - 20 - offset]{};
    }

    message(const message &right) {
        this->buffer = new uint8_t[0xffff - 20];
        int offset = 0;
        this->length = new(this->buffer) msg_len_type(*right.length);
        offset += sizeof(msg_len_type);
        this->id = new(this->buffer + offset) user_id_type(*right.id);
        offset += sizeof(user_id_type);
        this->flags = new(this->buffer + offset) flag_type(*right.flags);
        offset += sizeof(flag_type);
        this->msg = new(this->buffer + offset) uint8_t[0xffff - 20 - offset]{};
        memcpy(this->msg, right.msg, *this->length);
    }

    message(message &&right)  noexcept {
        this->buffer = std::exchange(right.buffer, nullptr);
        this->length = std::exchange(right.length, nullptr);
        this->id = std::exchange(right.id, nullptr);
        this->flags = std::exchange(right.flags, nullptr);
        this->msg = std::exchange(right.msg, nullptr);;
    }

    message(msg_len_type len, char *str, user_id_type id = -1, flag_type flags = 0) {
        this->buffer = new uint8_t[0xffff - 20];
        int offset = 0;
        this->length = new(this->buffer) msg_len_type(len);
        offset += sizeof(msg_len_type);
        this->id = new(this->buffer + offset) user_id_type(id);
        offset += sizeof(user_id_type);
        this->flags = new (this->buffer + offset) flag_type (flags);
        offset += sizeof(flag_type);
        this->msg = new(this->buffer + offset) uint8_t[0xffff - 20 - offset]{};
        memcpy(this->msg, str, *this->length);
    }

    ~message() {
        delete[] this->buffer;
    }

    friend std::ostream &operator<<(std::ostream &os, message &m);

//    friend std::istream &operator>>(std::istream &is, message &m);

    message &operator=(const message &right) {

        if (this == &right) {
            return *this;
        }
        int offset = 0;
        *this->length = *right.length;
        offset += sizeof(msg_len_type);
        *this->id = *right.id;
        offset += sizeof(user_id_type);
        *this->flags = *right.flags;
        offset += sizeof(flag_type);
        memcpy(this->msg, right.msg, *this->length);
        return *this;
    }

    char *operator+() const {
        return (char *) this->msg;
    }

    msg_len_type get_msg_size() const{
        return sizeof(msg_len_type) + sizeof(user_id_type) + sizeof(flag_type) + *this->length;
    }
};

std::ostream &operator<<(std::ostream &os, message &m) {
    os << (char *) m.msg;
    return os;
}

//std::istream &operator>>(std::istream &is, message &m) {
//    is >> *m.msg;
//    *m.length = strlen((char *) m.msg);
//    return is;
//}


#endif //MY_PROJECT_MESSAGE_H
