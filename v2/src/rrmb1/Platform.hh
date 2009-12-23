#ifndef MB_PLATFORM_ATMEGA644P_PLATFORM_HH_
#define MB_PLATFORM_ATMEGA644P_PLATFORM_HH_

#include <avr/interrupt.h>

#define UART_COUNT 2
#define HAS_COMMAND_QUEUE 1
#define HAS_PSU 1

//x axis pins
#define X_STEP_PIN      Pin(PortD,7)
#define X_DIR_PIN       Pin(PortC,2)
#define X_ENABLE_PIN    Pin(PortC,3)
#define X_MIN_PIN       Pin(PortC,4)
#define X_MAX_PIN       Pin(PortC,5)

//y axis pins
#define Y_STEP_PIN      Pin(PortC,7)
#define Y_DIR_PIN       Pin(PortC,6)
#define Y_ENABLE_PIN    Pin(PortA,7)
#define Y_MIN_PIN       Pin(PortA,6)
#define Y_MAX_PIN       Pin(PortA,5)

//z axis pins
#define Z_STEP_PIN      Pin(PortA,4)
#define Z_DIR_PIN       Pin(PortA,3)
#define Z_ENABLE_PIN    Pin(PortA,2)
#define Z_MIN_PIN       Pin(PortA,1)
#define Z_MAX_PIN       Pin(PortA,0)


#endif // MB_PLATFORM_ATMEGA644P_PLATFORM_HH_
