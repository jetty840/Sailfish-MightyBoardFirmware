#ifndef MB_CIRCULAR_BUFFER_HH_
#define MB_CIRCULAR_BUFFER_HH_

#include <stdint.h>

typedef uint16_t BufSizeType;
typedef uint8_t BufDataType;

/// A simple, reliable circular buffer implementation.
/// This implementation does not offer any protection from
/// interrupts and code writing over each other!  You must
/// disable interrupts before all accesses and writes to
/// a circular buffer that is updated in an interrupt (for
/// example, the UART implementations).

/// It's highly recommended that all accesses to circular
/// buffers be done through circular buffer cursors, which
/// ensure that interrupts are properly disabled and
/// enabled.

class CircularBuffer {
private:
	const BufSizeType size_; /// Size of this buffer
	volatile BufSizeType length_; /// Current length of valid buffer data
	volatile BufSizeType start_; /// Current start point of valid bufffer data
	volatile BufDataType* data_; /// Pointer to buffer data
	volatile bool overflow_; /// Overflow indicator
	volatile bool underflow_; /// Underflow indicator
public:
	CircularBuffer(BufSizeType size, BufDataType* data) :
		size_(size), length_(0), start_(0), data_(data), overflow_(false),
				underflow_(false) {
	}

	/// Reset the buffer to its empty state.  All data in
	/// the buffer will be (effectively) lost.
	inline void reset() {
		length_ = 0;
		overflow_ = false;
		underflow_ = false;
	}
	/// Append a byte to the tail of the buffer
	inline void push(BufDataType b) {
		if (length_ < size_) {
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
		const volatile BufDataType& popped_byte = operator[](0);
		start_ = (start_ + 1) % size_;
		length_--;
		return popped_byte;
	}

	/// Pop a number of bytes off the head of the buffer.  If there
	/// are not enough bytes to complete the pop, pop what we can and
	/// set the underflow flag.
	inline void pop(BufSizeType sz) {
		if (length_ < sz) {
			underflow_ = true;
			sz = length_;
		}
		start_ = (start_ + sz) % size_;
		length_ -= sz;
	}

	/// Get the length of the buffer
	inline const BufSizeType getLength() const {
		return length_;
	}
	/// Check if the buffer is empty
	inline const bool isEmpty() const {
		return length_ == 0;
	}
	/// Read the buffer directly
	inline volatile BufDataType& operator[](BufSizeType index) {
		const BufSizeType actual_index = (index + start_) % size_;
		return data_[actual_index];
	}
	/// Check the overflow flag
	inline const bool hasOverflow() const {
		return overflow_;
	}
	/// Check the underflow flag
	inline const bool hasUnderflow() const {
		return underflow_;
	}
};

#define DECLARE_BUFFER(name,size) \
extern CircularBuffer name(size,name##_data);

#define DEFINE_BUFFER(name,size) \
BufDataType name##_data[size]; \
CircularBuffer name(size,name##_data);

#endif // MB_CIRCULAR_BUFFER_HH_
