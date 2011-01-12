// Yep, this is actually -*- c++ -*-
/*********************************************************************************************************
 *  RepRap 3rd Generation Firmware (R3G)
 *
 *  Slave Extruder Firmware for Extruder Controller v2.x
 *
 *  Board documentation at: http://make.rrrf.org/ec-2.0
 *  Specification for this protocol is located at: http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  License: GPLv2
 *  Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 *
 *  Version History:
 *
 *  0001: Initial release of the protocol and firmware.
 *
 *********************************************************************************************************/

#if defined(__AVR_ATmega168__)
// okay
#elif defined(__AVR_ATmega328P__)
// okay
#else
#error Oops!  Make sure you have 'Arduino' selected from the boards menu.
#endif

//include some basic libraries.
#include <WProgram.h>
#include <stdint.h>
#include <EEPROM.h>
#include <SimplePacket.h>

#include "Configuration.h"
#ifdef HAS_SERVOS
#include <Servo.h>
#endif
#include "Datatypes.h"
#include "Variables.h"
#include "PacketProcessor.h"
#include "Extruder.h"
#include "Heater.h"
#include "ThermistorTable.h"

void init_serial();
void initialize();

//set up our firmware for actual usage.
void setup()
{
  //setup our firmware to a default state.
  init_serial(); //dont want to re-initialize serial!
  initialize();

  //this is a simple text string that identifies us.
  //Serial.print("R3G Slave v");
  //Serial.println(FIRMWARE_VERSION, DEC);
}

//this function takes us back to our default state.
void initialize()
{
  pinMode(DEBUG_PIN, OUTPUT);
  digitalWrite(DEBUG_PIN, LOW);
  is_tool_paused = false;
  initThermTable();
  init_extruder();
}

//start our hardware serial drivers
void init_serial()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  digitalWrite(RX_ENABLE_PIN, LOW); //always listen

  Serial.begin(SERIAL_SPEED);
}

//handle various things we're required to do.
void loop()
{
  //check for and handle any packets that come in.
  process_packets();

  //did we trigger a reversal?
  if (motor1_reversal_state)
    reverse_motor_1();

  //manage our extruder stuff.
  if (!is_tool_paused)
  {
    extruder_heater.manage_temperature();
    
#ifdef HAS_HEATED_BUILD_PLATFORM
    platform_heater.manage_temperature();
#endif

  }
}

//handle the abortion of a print job
void abort_print()
{
  //TODO: shut down all things

  //initalize everything to the beginning
  initialize();
}
