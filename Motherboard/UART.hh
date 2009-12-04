#ifndef MB_UART_HH_
#define MB_UART_HH_

typedef uint16_t BufSizeType;

/// Uart buffers are circular buffers.
struct UartBuffer {
    const BufSizeType size;    /// Size of this buffer
    BufSizeType length;        /// Current length of valid buffer data
    BufSizeType start;         /// Current start point of valid bufffer data
    uint8_t* data;            /// Dummy pointer

    /// Reset the buffer to its empty state
    inline void reset() { length = 0; }
    /// Append a byte to the buffer
    inline void append(uint_8 b) { if (length<size) data[length++] = b; }
    /// Manipulate the buffer directly
    inline uint8_t& operator[](size_t index) { return data[index]; }
};

class Uart {
private:
    UartBuffer& in_buffer_;
    UartBuffer& out_buffer_;
    // True if sending buffer on uart
    bool sending_;
    BufSizeType send_next_cursor_;
public:
    Uart(UartBuffer& in_buffer, UartBuffer& out_buffer) :
        in_buffer_(in_buffer), out_buffer_(out_buffer),
        sending_(false), send_next_cursor(0) {}
    /// Check to see if we are in the middle of sending a buffer's
    /// contents.
    inline bool isSending() { return sending_; }
    /// Get the next byte to send, and increment the sending cursor.
    inline uint8_t getNextSendingByte() {
        uint8_t& next_byte = out_buffer_[send_next_cursor_++];
        if (send_next_cursor_ >= out_buffer_.length) {
            sending_ = false;
        }
    }
    /// Indicate that we're ready to send the output buffer.
    void startSend();



#endif // #define MB_UART_HH_
