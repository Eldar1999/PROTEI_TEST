#include <gtest/gtest.h>

#include <string>

#include "circle_queue.h"
#include "message.h"


TEST(test, default_constructor_test) {
    circle_queue a;
    EXPECT_EQ(a.buff_size, 0x10);
    EXPECT_EQ(a.free_space, a.buff_size);
    EXPECT_EQ(a.payload, a.buff_size - a.free_space);
    EXPECT_EQ(a.read_pos, 0);
    EXPECT_EQ(a.append_pos, 0);
    EXPECT_EQ(a.to_read, false);
    EXPECT_EQ(a.buff[0], 0);
    EXPECT_EQ(a.buff[a.buff_size - 1], 0);
}

TEST(test, custom_constructor_test) {
    unsigned long len = random() % (UINT16_MAX - 100) + 100;
    circle_queue a(len);
    EXPECT_EQ(a.buff_size, len);
    EXPECT_EQ(a.free_space, a.buff_size);
    EXPECT_EQ(a.payload, a.buff_size - a.free_space);
    EXPECT_EQ(a.read_pos, 0);
    EXPECT_EQ(a.append_pos, 0);
    EXPECT_EQ(a.to_read, false);
    EXPECT_EQ(a.buff[0], 0);
    EXPECT_EQ(a.buff[a.buff_size - 1], 0);
}

TEST(test, destructor_test) {
    auto *a = new circle_queue();
    EXPECT_NO_FATAL_FAILURE(delete a);
}

TEST(test, push_byte_test) {
    circle_queue a;
    a.push(1, (uint8_t *) "a");
    EXPECT_EQ(a.buff[0], 'a');
    EXPECT_EQ(a.payload, 1);
    EXPECT_EQ(a.free_space, a.buff_size - a.payload);
    EXPECT_EQ(a.append_pos, 1);
}

TEST(test, pop_byte_test) {
    circle_queue a;
    a.push(1, (uint8_t *) "a");
    uint8_t tmp;
    a.pop(1, &tmp);
    EXPECT_EQ(tmp, 'a');
    EXPECT_EQ(a.payload, 0);
    EXPECT_EQ(a.free_space, a.buff_size - a.payload);
    EXPECT_EQ(a.read_pos, 1);
}

TEST(test, push_data_bigger_than_buffer) {
    circle_queue a;
    EXPECT_EQ(a.push(30, (uint8_t *) "ABOBABOBABOBABOBABOBABOBABOBABOBA"), -1);
    EXPECT_EQ(a.append_pos, 0);
}

TEST(test, pop_data_bigger_than_payload) {
    circle_queue a;
    EXPECT_EQ(a.pop(1, nullptr), -1);
    EXPECT_EQ(a.append_pos, 0);
}

TEST(test, push_data_sizeof_buffsize) {
    circle_queue a;
    EXPECT_NO_FATAL_FAILURE(a.push(0x10, (uint8_t *) "1234567890abcdef"));
    EXPECT_EQ(a.buff[0], '1');
    EXPECT_EQ(a.buff[a.buff_size - 1], 'f');
    EXPECT_EQ(a.payload, a.buff_size);
    EXPECT_EQ(a.free_space, a.buff_size - a.payload);
    EXPECT_EQ(a.append_pos, 0x10);
}

TEST(test, push_pop_data_sizeof_buffsize) {
    circle_queue a;
    uint8_t tmp1[0x10];
    for (int i = 0; i < 10; i++) {
        tmp1[i] = random() % UINT8_MAX;
    }
    uint8_t tmp2[0x10];
    a.push(0x10, tmp1);
    EXPECT_NO_FATAL_FAILURE(a.pop(0x10, tmp2));
    EXPECT_EQ(memcmp(tmp1, tmp2, 0x10), 0);
    EXPECT_EQ(a.payload, 0);
    EXPECT_EQ(a.free_space, a.buff_size - a.payload);
    EXPECT_EQ(a.read_pos, 0x10);
}

TEST(test, circle_push_pop_data_sizeof_buffsize) {
    circle_queue a;
    uint8_t tmp1[0x10];
    for (int i = 0; i < 10; i++) {
        tmp1[i] = random() % UINT8_MAX;
    }
    uint8_t tmp2[0x10];
    for (int i = 0; i < 10; i++) {
        a.push(0x10, tmp1);
        EXPECT_NO_FATAL_FAILURE(a.pop(0x10, tmp2));
        EXPECT_EQ(memcmp(tmp1, tmp2, 0x10), 0);
        EXPECT_EQ(a.payload, 0);
        EXPECT_EQ(a.free_space, a.buff_size - a.payload);
        EXPECT_EQ(a.read_pos, 0x10);
    }
}

TEST(test, circle_push_pop_data_sizeof_randsize) {
    circle_queue a;
    uint8_t tmp1[0x10];
    for (int i = 0; i < 10; i++) {
        tmp1[i] = random() % UINT8_MAX;
    }
    uint8_t tmp2[0x10];
    unsigned long size;
    uint64_t readpos = 0;
    for (int i = 0; i < 100; i++) {
        size = random() % (a.buff_size - 1) + 1;
        readpos = readpos + size > a.buff_size ? readpos + size - a.buff_size : readpos + size;
        a.push(size, tmp1);
        EXPECT_NO_FATAL_FAILURE(a.pop(size, tmp2));
        EXPECT_EQ(memcmp(tmp1, tmp2, size), 0);
        EXPECT_EQ(a.payload, 0);
        EXPECT_EQ(a.free_space, a.buff_size - a.payload);
        EXPECT_EQ(a.read_pos, readpos);
    }
}

TEST(test, message_type_default_constructor) {
    message m;
    EXPECT_EQ(*m.length, 0);
    EXPECT_EQ(m.msg[0], 0);
}

TEST(test, message_type_custom_constructor) {
    message m(7, (char *) "Lazyboy");
    EXPECT_EQ(*m.length, 7);
    EXPECT_EQ(memcmp(m.msg, "Lazyboy", 7), 0);
}

TEST(test, message_copy_constructor) {
    message m(7, (char *) "Lazyboy");
    message m1 = m;
    EXPECT_EQ(*m1.length, 7);
    EXPECT_EQ(memcmp(m1.msg, "Lazyboy", 7), 0);
}

TEST(test, message_copy_operator) {
    message m(7, (char *) "Lazyboy");
    message m1;
    m1 = m;
    EXPECT_EQ(*m1.length, 7);
    EXPECT_EQ(memcmp(m1.msg, "Lazyboy", 7), 0);
}

TEST(test, message_copy_operator_selfcopy) {
    message m(7, (char *) "Lazyboy");
    EXPECT_EQ(&m, &(m = m));
}

TEST(test, message_ostream_operator) {
    message m(7, (char *) "Lazyboy");
    EXPECT_NO_FATAL_FAILURE(std::cout << m << std::endl);
}

TEST(test, message_istream_operator) {
    message m;
    EXPECT_NO_FATAL_FAILURE(std::istringstream("blablabla") >> m);
    EXPECT_EQ(*m.length, 9);
    EXPECT_EQ(memcmp(m.msg, "blablabla", 9), 0);
}