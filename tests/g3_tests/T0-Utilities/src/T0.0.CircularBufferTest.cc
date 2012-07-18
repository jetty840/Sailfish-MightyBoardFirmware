#include <gtest/gtest.h>
#include "CircularBuffer.hh"

const BufSizeType buffer_size = 29;

TEST(CircularBufferTest, WalkAround) {
    DEFINE_BUFFER(cb,uint8_t,buffer_size);
    for (int offset = 0; offset < buffer_size*3; offset++) {
        ASSERT_EQ(cb.getLength(),0);
        cb.push(offset);
        ASSERT_EQ(cb.getLength(),1);
        ASSERT_EQ(cb[0],offset);
        ASSERT_EQ(cb.pop(),offset);
    }
    ASSERT_FALSE(cb.hasOverflow());
    ASSERT_FALSE(cb.hasUnderflow());
}

TEST(CircularBufferTest, LoopExerciser) {
    DEFINE_BUFFER(cb,uint8_t,buffer_size);
    // The trick here is to completely push the buffer full of data, pop it empty,
    // push and pop one more time to advance the start, and repeat.
    for (int offset = 0; offset < buffer_size*2; offset++) {
        int fill_count;
        // fill, check, and empty buffer
        for (fill_count = 0; fill_count < buffer_size; fill_count++) {
            cb.push(fill_count);
            ASSERT_EQ(cb.getLength(),fill_count+1);
            ASSERT_FALSE(cb.hasOverflow());
        }
        // check buffer contents
        for (fill_count = 0; fill_count < buffer_size; fill_count++) {
            ASSERT_EQ(cb[fill_count],fill_count);
        }
        // empty buffer
        for (fill_count = 0; fill_count < buffer_size; fill_count++) {
            ASSERT_EQ(cb.pop(),fill_count);
            ASSERT_EQ(cb.getLength(),buffer_size - (fill_count+1));
            ASSERT_FALSE(cb.hasUnderflow());
        }
        // advance buffer by one count
        ASSERT_EQ(cb.getLength(),0);
        cb.push(0xff);
        ASSERT_EQ(cb.pop(),0xff);
        ASSERT_FALSE(cb.hasOverflow());
        ASSERT_FALSE(cb.hasUnderflow());
    }
}

TEST(CircularBufferTest,OverflowCheck) {
    DEFINE_BUFFER(cb,uint8_t,buffer_size);
    // Advance start by one (via push/pop), fill the buffer to overflow,
    // and reset the buffer.  Cycle around more than one buffer size.
    for (int offset = 0; offset < buffer_size*2; offset++) {
        int fill_count;
        // fill, check, and empty buffer
        for (fill_count = 0; fill_count < buffer_size; fill_count++) {
            cb.push(fill_count);
            ASSERT_FALSE(cb.hasOverflow());
        }
        cb.push(1);
        ASSERT_TRUE(cb.hasOverflow());
        cb.reset();
        ASSERT_FALSE(cb.hasOverflow());
        ASSERT_EQ(cb.getLength(),0);
        // advance buffer by one count
        cb.push(0xff);
        ASSERT_EQ(cb.pop(),0xff);
        ASSERT_FALSE(cb.hasOverflow());
        ASSERT_FALSE(cb.hasUnderflow());
    }
}

TEST(CircularBufferTest,UnderflowCheck) {
    DEFINE_BUFFER(cb,uint8_t,buffer_size);
    // Advance start by one (via push/pop), fill the buffer to overflow,
    // and reset the buffer.  Cycle around more than one buffer size.
    for (int offset = 0; offset < buffer_size*2; offset++) {
        int fill_size = offset%3;
        int fill_count;
        // fill, empty, and check buffer
        for (fill_count = 0; fill_count < fill_size; fill_count++) {
            cb.push(0);
        }
        for (fill_count = 0; fill_count < fill_size; fill_count++) {
            cb.pop();
            ASSERT_FALSE(cb.hasUnderflow());
        }
        cb.pop();
        ASSERT_TRUE(cb.hasUnderflow());
        cb.reset();
        ASSERT_FALSE(cb.hasUnderflow());
        ASSERT_EQ(cb.getLength(),0);
        // advance buffer by one count
        cb.push(0xff);
        ASSERT_EQ(cb.pop(),0xff);
        ASSERT_FALSE(cb.hasOverflow());
        ASSERT_FALSE(cb.hasUnderflow());
    }
}
