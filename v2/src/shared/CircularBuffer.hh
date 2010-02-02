#ifndef MB_CIRCULAR_BUFFER_HH_
#define MB_CIRCULAR_BUFFER_HH_

#include <stdint.h>

typedef uint16_t BufSizeType;

/// A simple, reliable circular buffer implementation.
/// This implementation does not offer any protection from
/// interrupts and code writing over each other!  You must
/// disable interrupts before all accesses and writes to
/// a circular buffer that is updated in an interrupt.
template<typename T>
class CircularBufferTempl {
public:
	typedef T BufDataType;
private:
	const BufSizeType size_; /// Size of this buffer
	volatile BufSizeType length_; /// Current length of valid buffer data
	volatile BufSizeType start_; /// Current start point of valid bufffer data
	BufDataType* data_; /// Pointer to buffer data
	volatile bool overflow_; /// Overflow indicator
	volatile bool underflow_; /// Underflow indicator
public:
	CircularBufferTempl(BufSizeType size, BufDataType* data) :
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
			return BufDataType();
		}
		const BufDataType& popped_byte = operator[](0);
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

	/// Get the remaining capacity of this buffer
	inline const BufSizeType getRemainingCapacity() const {
		return size_ - length_;
	}

	/// Check if the buffer is empty
	inline const bool isEmpty() const {
		return length_ == 0;
	}
	/// Read the buffer directly
	inline BufDataType& operator[](BufSizeType index) {
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

typedef CircularBufferTempl<uint8_t> CircularBuffer;

#define DEFINE_BUFFER(name,dtype,size) \
dtype name##_data[size]; \
CircularBufferTempl<dtype> name(size,name##_data);

#endif // MB_CIRCULAR_BUFFER_HH_
