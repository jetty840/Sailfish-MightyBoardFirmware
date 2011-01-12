#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_
/**
 *  Sanguino 3rd Generation Firmware (S3G)
 *
 *  Specification for this protocol is located at: 
 *    http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  License: GPLv2
 *  Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 */

#include <stddef.h>
#include <stdint.h>

/**
 * This is an implementation of a simple in-memory circular
 * buffer.
 *
 * Please note that this implementation is NOT thread/interrupt
 * safe.  Remember that 16-bit data access is not atomic on
 * 8-bit platforms!  Turn off interrupts before accessing the
 * buffer.
 */
class CircularBuffer {
private:
  uint8_t* buffer;        // ptr to the in-memory buffer
  uint16_t capacity;      // size of the buffer
  uint16_t head;          // index of first element of data
  uint16_t tail;          // index of last element of data
  uint16_t currentSize;   // number of elements of valid data in buffer
public:
  /**
   * Create a circular buffer of the given size with a provided
   * chunk of memory.
   * This implementation does not claim ownership of the buffer!
   */
  CircularBuffer(uint16_t capacity, uint8_t* pBuf) {
    this->capacity = capacity;
    buffer = pBuf;
    clear();
  }

  ~CircularBuffer() {
  }

  /**
   * Reset the circular buffer to an empty state.
   */
  void clear() {
    head = tail = 0;
    currentSize = 0;
  }

  /**
   * Return a byte of data in the circular buffer specified by its index.
   */
  uint8_t operator[](uint16_t i) {
    uint16_t idx;
    idx = (i + tail) % capacity;
    return buffer[idx];
  }

  /**
   * Return the remaining capacity of the circular buffer.
   */
  uint16_t remainingCapacity()
  {
    uint16_t remaining;
    remaining = (capacity - currentSize);
    return remaining;
  }

  /**
   * Return the current number of valid bytes in the buffer.
   */
  uint16_t size() {
    uint16_t csize;
    csize = currentSize;
    return csize;
  }

private:
  /**
   * Internal: append a byte of data to the buffer's contents.
   */
  void appendInternal(uint8_t datum)
  {
    if ((currentSize + 1) <= capacity)
    {
      buffer[head] = datum;
      head = (head + 1) % capacity;
      currentSize++;
    }
  }
  
public:
  /**
   * Append a single byte of data to the buffer.
   */
  void append(uint8_t datum) {
    appendInternal(datum);
  }

  /**
   * Append a two-byte value to the buffer.
   * The buffer stores values in little-endian format.
   */
  void append_16(uint16_t datum) {
    appendInternal(datum & 0xff);
    appendInternal((datum >> 8) & 0xff);
  }

  /**
   * Append a four-byte value to the buffer.
   * The buffer stores values in little-endian format.
   */
  void append_32(uint32_t datum) {
    appendInternal(datum & 0xff);
    appendInternal((datum >> 8) & 0xff);
    appendInternal((datum >> 16) & 0xff);
    appendInternal((datum >> 24) & 0xff);
  }

private:
  /**
   * Internal: Remove the first byte of the buffer's data and return it.
   */
  inline uint8_t removeInternal() {
    if (currentSize == 0)
      return 0;
    else
    {
      uint8_t c = buffer[tail];
      tail = (tail + 1) % capacity;
      currentSize--;
      return c;
    }
  }

public:    
  /**
   * Remove and return a byte from the start of the circular buffer.
   */
  uint8_t remove_8() {
    return removeInternal();
  }

  /*
   * Remove and return a two-byte value  from the start of the circular 
   * buffer.  The buffer stores values in little-endian format.
   */
  uint16_t remove_16() {
    uint8_t v[2];
    v[0] = removeInternal();
    v[1] = removeInternal();
    return v[0] | ((uint16_t)v[1] << 8);
  }

  /*
   * Remove and return a four-byte value  from the start of the circular 
   * buffer.  The buffer stores values in little-endian format.
   */
  uint32_t remove_32() {
    uint8_t v[4];
    v[0] = removeInternal();
    v[1] = removeInternal();
    v[2] = removeInternal();
    v[3] = removeInternal();
    return v[0] |
      ((uint32_t)v[1] << 8) |
      ((uint32_t)v[2] << 16) |
      ((uint32_t)v[3] << 24);
  }

  void removeCount( uint16_t count ) {
    if (currentSize < count)
      count = currentSize;
    tail = (tail + count) % capacity;
    currentSize -= count;
  }

  class Cursor {
  private:
    CircularBuffer& buffer;
    uint16_t idx;
  public:
    Cursor(CircularBuffer& src) : buffer(src), idx(0) {}
    uint8_t read_8() {
      return buffer[idx++];
    }
    uint16_t read_16() {
      return 
	((uint16_t)read_8() << 0) |
	((uint16_t)read_8() << 8);
    }
    uint32_t read_32() {
      return 
	((uint32_t)read_8() << 0) |
	((uint32_t)read_8() << 8) |
	((uint32_t)read_8() << 16) |
	((uint32_t)read_8() << 24);
    }
    void commit() {
      buffer.removeCount(idx);
    }
    void rollback() {
      idx = 0;
    }
  };

  Cursor newCursor() {
    return Cursor(*this);
  }

};

#endif // _CIRCULAR_BUFFER_H_
