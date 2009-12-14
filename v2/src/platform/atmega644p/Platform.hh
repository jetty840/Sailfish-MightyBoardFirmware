#ifndef MB_PLATFORM_ATMEGA644P_PLATFORM_HH_
#define MB_PLATFORM_ATMEGA644P_PLATFORM_HH_

#include <avr/interrupt.h>

class DisableInterrupts() {
public:
    DisableInterrupts() {
    	cli();
    }
    ~DisableInterrupts() {
    	sei();
    }
};

#endif // MB_PLATFORM_ATMEGA644P_PLATFORM_HH_
