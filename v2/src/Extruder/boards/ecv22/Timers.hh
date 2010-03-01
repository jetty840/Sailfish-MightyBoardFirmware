#ifndef MOTHERBOARD_BOARD_RRMBV12_TIMERS
#define MOTHERBOARD_BOARD_RRMBV12_TIMERS

#include <stdint.h>

void startTimers();

typedef uint32_t micros_t;

micros_t getCurrentMicros();

#endif // MOTHERBOARD_BOARD_RRMBV12_TIMERS
