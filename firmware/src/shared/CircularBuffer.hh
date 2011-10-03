/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */


#ifndef SHARED_CIRCULAR_BUFFER_HH_
#define SHARED_CIRCULAR_BUFFER_HH_

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
	const BufSizeType size; /// Size of this buffer
	volatile BufSizeType length; /// Current length of valid buffer data
	volatile BufSizeType start; /// Current start point of valid bufffer data
	BufDataType* const data; /// Pointer to buffer data
	volatile bool overflow; /// Overflow indicator
	volatile bool underflow; /// Underflow indicator
public:
	CircularBufferTempl(BufSizeType size_in, BufDataType* data_in) :
		size(size_in), length(0), start(0), data(data_in), overflow(false),
				underflow(false) {
	}

	/// Reset the buffer to its empty state.  All data in
	/// the buffer will be (effectively) lost.
	inline void reset() {
		length = 0;
		start = 0;
		overflow = false;
		underflow = false;
	}
	/// Append a byte to the tail of the buffer
	inline void push(BufDataType b) {
		if (length < size) {
			operator[](length) = b;
			length++;
		} else {
			overflow = true;
		}
	}
	/// Pop a byte off the head of the buffer
	inline BufDataType pop() {
		if (isEmpty()) {
			underflow = true;
			return BufDataType();
		}
		const BufDataType& popped_byte = operator[](0);
		start = (start + 1) % size;
		length--;
		return popped_byte;
	}

	/// Pop a number of bytes off the head of the buffer.  If there
	/// are not enough bytes to complete the pop, pop what we can and
	/// set the underflow flag.
	inline void pop(BufSizeType sz) {
		if (length < sz) {
			underflow = true;
			sz = length;
		}
		start = (start + sz) % size;
		length -= sz;
	}

	/// Get the length of the buffer
	inline const BufSizeType getLength() const {
		return length;
	}

	/// Get the remaining capacity of this buffer
	inline const BufSizeType getRemainingCapacity() const {
		return size - length;
	}

	/// Check if the buffer is empty
	inline const bool isEmpty() const {
		return length == 0;
	}
	/// Read the buffer directly
	inline BufDataType& operator[](BufSizeType index) {
		const BufSizeType actual_index = (index + start) % size;
		return data[actual_index];
	}
	/// Check the overflow flag
	inline const bool hasOverflow() const {
		return overflow;
	}
	/// Check the underflow flag
	inline const bool hasUnderflow() const {
		return underflow;
	}
};

typedef CircularBufferTempl<uint8_t> CircularBuffer;

#define DEFINE_BUFFER(name,dtype,size) \
dtype name##_data[size]; \
CircularBufferTempl<dtype> name(size,name##_data);

#endif // SHARED_CIRCULAR_BUFFER_HH_
