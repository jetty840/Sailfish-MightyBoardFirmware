#include <stdint.h>

class CommStats {
public:
  static uint16_t badEchos;
  static uint16_t crcFailures;
  static uint16_t packetTimeouts;
};
