#ifndef _SIMPLE_PACKET_H_
#define _SIMPLE_PACKET_H_

//include our various libraries.
#include <util/crc16.h>
#include <stdint.h>
#include "HardwareSerial.h"

#define START_BYTE 0xD5
#define MAX_PACKET_LENGTH 32

typedef void (*txFuncPtr)(uint8_t);

// packet states
typedef enum {
  PS_START = 0,
  PS_LEN,
  PS_PAYLOAD,
  PS_CRC,
  PS_LAST
} 
PacketState;

// various error codes
typedef enum {
  RC_GENERIC_ERROR   = 0,
  RC_OK              = 1,
  RC_BUFFER_OVERFLOW = 2,
  RC_CRC_MISMATCH    = 3,
  RC_PACKET_TOO_BIG  = 4,
  RC_CMD_UNSUPPORTED = 5
} 
ResponseCode;

class SimplePacket {
private:
  //variables for our incoming packet.
  PacketState state;
  uint8_t target_length;
  uint8_t rx_length;
  uint8_t rx_data[MAX_PACKET_LENGTH];
  uint8_t rx_crc;
  uint8_t tx_length;
  uint8_t tx_data[MAX_PACKET_LENGTH];
  uint8_t tx_crc;
  ResponseCode response_code;

  txFuncPtr txFunc;

public:
  const static int PACKET_OVERHEAD = 3;
  SimplePacket(txFuncPtr myPtr);
  void init();

  //process a byte from our packet
  void process_byte(uint8_t b);
  bool isFinished();
  bool isStarted() { return state != PS_START; }
  uint8_t getLength();
  PacketState getState();
  ResponseCode getResponseCode();

  void unsupported();
  void overflow();

  void sendReply();
  void sendPacket();
  void transmit(uint8_t d);

  void add_32(uint32_t d);
  void add_16(uint16_t d);
  void add_8(uint8_t d);

  uint8_t get_8(uint8_t idx);
  uint16_t get_16(uint8_t idx);
  uint32_t get_32(uint8_t idx);
};

#endif // _SIMPLE_PACKET_H_
