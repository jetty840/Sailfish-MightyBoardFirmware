#ifndef MB_CIRCULAR_BUFFER_HH_
#define MB_CIRCULAR_BUFFER_HH_

#include <stdint.h>

typedef uint16_t BufSizeType;
typedef uint8_t BufDataType;

/// A simple, reliable circular buffer implementation.
class CircularBuffer {
private:
    const BufSizeType size_;    /// Size of this buffer
    BufSizeType length_;        /// Current length of valid buffer data
    BufSizeType start_;         /// Current start point of valid bufffer data
    BufDataType* data_;         /// Pointer to buffer data
    bool overflow_;             /// Overflow indicator
    bool underflow_;            /// Underflow indicator
public:
    CircularBuffer(BufSizeType size, BufDataType* data) :
        size_(size), length_(0), start_(0), data_(data),
        overflow_(false), underflow_(false) {}

    /// Reset the buffer to its empty state.  All data in
    /// the buffer will be (effectively) lost.
    inline void reset() {
        length_ = 0;
        overflow_ = false;
        underflow_ = false;
    }
    /// Append a byte to the tail of the buffer
    inline void push(BufDataType b) {
        if (length_<size_) {
            operator[](length_) = b;
            length_++;
        } else {
            overflow_ = true;
        }
    }
    /// Pop a byte off the head of the buffer
    inline BufDataType pop() {
        if (isEmpty()) {
            underflow_ = true;
            return 0;
        }
        const BufDataType& popped_byte = operator[](0);
        start_ = (start_+1) % size_;
        length_--;
        return popped_byte;
    }
    /// Get the length of the buffer
    inline const BufSizeType getLength() const { return length_; }
    /// Check if the buffer is empty
    inline const isEmpty() const { return length_ == 0; }
    /// Read the buffer directly
    inline const BufDataType& operator[](BufSizeType index) const {
        const BufSizeType actual_index = (index + start_) % size_;
        return data[actual_index];
    }
};

#endif // MB_CIRCULAR_BUFFER_HH_
