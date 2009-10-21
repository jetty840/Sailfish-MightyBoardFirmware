// Yep, this is actually -*- c++ -*-
/***************************************************************************************
 *  Sanguino 3rd Generation Firmware (S3G)
 *
 *  Specification for this protocol is located at: 
 *    http://docs.google.com/Doc?id=dd5prwmp_14ggw37mfp
 *  
 *  License: GPLv2
 *  Authors: Marius Kintel, Adam Mayer, and Zach Hoeken
 *
 ***************************************************************************************/

//a check to make sure we're compiling for the right firmware
#ifndef __AVR_ATmega644P__
#error Oops!  Make sure you have 'Sanguino' selected from the 'Tools -> Boards' menu.
#endif

//include some basic libraries.
#include <WProgram.h>
#include <SimplePacket.h>
#include <EEPROM.h>

#include "Configuration.h"
#include "Datatypes.h"
#include "CircularBuffer.h"
#include "RS485.h"
#include "Variables.h"
#include "Commands.h"
#include "SDSupport.h"
#include "Steppers.h"
#include "Tools.h"
#include "Version.h"
#include "PSU.h"
#include "PacketProcessor.h"

#ifdef USE_SD_CARD
#include <RepRapSDCard.h>
#endif

void init_serial();
void initialize();

//set up our firmware for actual usage.
void setup()
{
  //setup our firmware to a default state.
  init_serial();
  initialize();

  //this is a simple text string that identifies us.
  Serial.print("R3G Master v");
  Serial.println(FIRMWARE_VERSION, DEC);
}

//this function takes us back to our default state.
void initialize()
{
  is_machine_paused = false;

  init_psu();
  init_commands();
  init_steppers();
  init_tools();
  sd_reset();
}

//start our hardware serial drivers
void init_serial()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  digitalWrite(RX_ENABLE_PIN, LOW); //always listen.

  Serial.begin(HOST_SERIAL_SPEED);
  Serial1.begin(SLAVE_SERIAL_SPEED);
}

//handle various things we're required to do.
void loop()
{
  //check for and handle any packets that come in.
  if (Serial.available())
    process_host_packets();

  //our basic handling for each loop.
  if (commandMode == COMMAND_MODE_IDLE)
    handle_commands();
  else if (commandMode == COMMAND_MODE_WAIT_FOR_TOOL)
    check_tool_ready_state();
}

// prototype of fn defined in Tools.pde
void abort_current_tool();

//handle the abortion of a print job
void abort_print()
{
  //turn off all of our tools.
  abort_current_tool();

  //turn off steppers too.
  pause_stepping();

  //initalize everything to the beginning
  initialize();
}
