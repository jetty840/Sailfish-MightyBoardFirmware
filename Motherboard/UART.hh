#ifndef MB_UART_HH_
#define MB_UART_HH_

#include "Processor.hh"
#include "CircularBuffer.hh"

/// Uart buffers are circular buffers.
typedef CircularBuffer UartBuffer;

typedef void (*sendTrigger)();

/// An abstract UART class.  Note that this implentation does not directly
/// access any processor features; instead it implements a simple, fast
/// interface meant to be called by interrupt processors, etc.
/// Be sure to disable interrupts before accessing the circular input and
/// output buffers.
///
/// Usage:
/// The input buffer should be automatically filled by the UART's RX ISR.
/// Once a send condition is indicated, the output buffer will be emptied
/// by the UART's TX ISR.  The output buffer will record the size of the
/// buffer at the time a send is triggered, and only send the data that is
/// in the buffer at send time.
class Uart {
private:
    UartBuffer in_buffer_;
    UartBuffer out_buffer_;
    // True if sending buffer on uart
    bool sending_;
    BufSizeType send_next_cursor_;
    BufSizeType send_remaining_chars_;
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
        if (send_remaining_chars_ <= 0) {
            sending_ = false;
        }
    }
    /// Indicate that we're ready to send the output buffer.
    inline void startSend() {
        send_remaining_chars_ = out_buffer_.getLength();
        sending_ = true;
        send_trigger_();
    }

    /// Get a reference to the input buffer.
    UartBuffer& in() { return in_buffer_; }
    UartBuffer& out() { return out_buffer_; }

};

#define DEFINE_UART(


#endif // #define MB_UART_HH_
