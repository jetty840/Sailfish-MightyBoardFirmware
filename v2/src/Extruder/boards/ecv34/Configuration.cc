#include "Configuration.hh"
#include "AvrPort.hh"

Pin TX_ENABLE_PIN(PortC, 0);
Pin RX_ENABLE_PIN(PortC, 1);

Pin CHANNEL_A(PortB,2);
Pin CHANNEL_B(PortB,1);
Pin CHANNEL_C(PortD,6);

Pin MOTOR_ENABLE_PIN(PortD,5);
Pin MOTOR_DIR_PIN(PortB,0);

Pin THERMOCOUPLE_CS(PortD,4);
Pin THERMOCOUPLE_SCK(PortB,5);
Pin THERMOCOUPLE_SO(PortB,4);


Pin SERVO0(PortC,2);
Pin SERVO1(PortC,3);
