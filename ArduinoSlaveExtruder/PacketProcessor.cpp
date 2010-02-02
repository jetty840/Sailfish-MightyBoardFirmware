#include <EEPROM.h>
#include <SimplePacket.h>
#include "RS485.h"
#include "Variables.h"
#include "Configuration.h"
#include "Utils.h"
#include "Version.h"
#include "ArduinoSlaveExtruder.h"
#include "Extruder.h"

//these are our packet classes
SimplePacket masterPacket(rs485_tx);

// Yep, this is actually -*- c++ -*-
// These are our query commands from the host
#define SLAVE_CMD_VERSION                0
#define SLAVE_CMD_INIT                   1
#define SLAVE_CMD_GET_TEMP               2
#define SLAVE_CMD_SET_TEMP               3
#define SLAVE_CMD_SET_MOTOR_1_PWM        4
#define SLAVE_CMD_SET_MOTOR_2_PWM        5
#define SLAVE_CMD_SET_MOTOR_1_RPM        6
#define SLAVE_CMD_SET_MOTOR_2_RPM        7
#define SLAVE_CMD_SET_MOTOR_1_DIR        8
#define SLAVE_CMD_SET_MOTOR_2_DIR        9
#define SLAVE_CMD_TOGGLE_MOTOR_1        10
#define SLAVE_CMD_TOGGLE_MOTOR_2        11
#define SLAVE_CMD_TOGGLE_FAN            12
#define SLAVE_CMD_TOGGLE_VALVE          13
#define SLAVE_CMD_SET_SERVO_1_POS       14
#define SLAVE_CMD_SET_SERVO_2_POS       15
#define SLAVE_CMD_FILAMENT_STATUS       16
#define SLAVE_CMD_GET_MOTOR_1_PWM       17
#define SLAVE_CMD_GET_MOTOR_2_PWM       18
#define SLAVE_CMD_GET_MOTOR_1_RPM       19
#define SLAVE_CMD_GET_MOTOR_2_RPM       20
#define SLAVE_CMD_SELECT_TOOL           21
#define SLAVE_CMD_IS_TOOL_READY         22
#define SLAVE_CMD_PAUSE_UNPAUSE         23
#define SLAVE_CMD_ABORT                 24
#define SLAVE_CMD_READ_FROM_EEPROM      25
#define SLAVE_CMD_WRITE_TO_EEPROM       26

#define SLAVE_CMD_GET_PLATFORM_TEMP     30
#define SLAVE_CMD_SET_PLATFORM_TEMP     31

void handle_query();
void send_reply();

//initialize the firmware to default state.
void init_commands()
{
  masterPacket.init();
}

//handle our packets.
void process_packets()
{
  //do we have any data to process?
  if (Serial.available() > 0)
  {
    unsigned long start = millis();
    unsigned long end = start + PACKET_TIMEOUT;

    //is there serial data available?
    //read through our available data
    while (!masterPacket.isFinished())
    {
      //only try to grab it if theres something in the queue.
      if (Serial.available() > 0)
      {
        //digitalWrite(DEBUG_PIN, HIGH);

        //grab a byte and process it.
        byte d = Serial.read();
        masterPacket.process_byte(d);

        //keep us goign while we have data coming in.
        start = millis();
        end = start + PACKET_TIMEOUT;

        //digitalWrite(DEBUG_PIN, LOW);
      }

      //are we sure we wanna break mid-packet?
      //have we timed out?
      if (millis() >= end)
        return;
    }

    //do we have a finished packet?
    if (masterPacket.isFinished())
    {
      //only process packets intended for us.
      if (masterPacket.get_8(0) == RS485_ADDRESS)
      {
        //take some action.
        handle_query();                

        //send reply over RS485
        send_reply();

        //okay, we'll come back later.
        return;
      }
    }

    //always clean up the packet.
    masterPacket.init();  
  }
}

void send_reply()
{
  //might be needed to allow for pin switching / etc.  testing needed.
  // TODO: evaluate
  // delayMicrosecondsInterruptible(50);
  
  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx

  // TODO: evaluate
  //delayMicrosecondsInterruptible(10);

  //okay, send our response
  masterPacket.sendReply();

  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx
}

//this is for handling query commands that need a response.
void handle_query()
{
  byte temp;

  //which one did we get?
  switch (masterPacket.get_8(1))
  {
  //WORKING
  case SLAVE_CMD_VERSION:
    //get our host version
    master_version = masterPacket.get_16(2);

    //send our version back.
    masterPacket.add_16(FIRMWARE_VERSION);
    break;

  //WORKING
  case SLAVE_CMD_INIT:
    //just initialize
    initialize();
    break;

  //WORKING
  case SLAVE_CMD_GET_TEMP:
    masterPacket.add_16(extruder_heater.get_current_temperature());
    break;

  //WORKING
  case SLAVE_CMD_SET_TEMP:
    extruder_heater.set_target_temperature(masterPacket.get_16(2));
    break;

#ifdef HAS_HEATED_BUILD_PLATFORM
  //WORKING
  case SLAVE_CMD_GET_PLATFORM_TEMP:
    masterPacket.add_16(platform_heater.get_current_temperature());
    break;

  //WORKING
  case SLAVE_CMD_SET_PLATFORM_TEMP:
    platform_heater.set_target_temperature(masterPacket.get_16(2));
    break;
#endif
    
  //WORKING
  case SLAVE_CMD_SET_MOTOR_1_PWM:
    motor1_control = MC_PWM;
    motor1_pwm = masterPacket.get_8(2);
    break;

  //WORKING
  case SLAVE_CMD_SET_MOTOR_2_PWM:
    motor2_control = MC_PWM;
    motor2_pwm = masterPacket.get_8(2);
    break;

  //NEEDS TESTING
  case SLAVE_CMD_SET_MOTOR_1_RPM:
    motor1_target_rpm = masterPacket.get_32(2) * 16;
    #if MOTOR_STYLE == 1
      motor1_control = MC_ENCODER;
    #else
      motor1_control = MC_STEPPER;
      stepper_ticks = motor1_target_rpm / (MOTOR_STEPS * MOTOR_STEP_MULTIPLIER);
      stepper_high_pwm = motor1_pwm;
      stepper_low_pwm = round((float)motor1_pwm * 0.4);
    #endif
    break;

  //NEEDS TESTING
  case SLAVE_CMD_SET_MOTOR_2_RPM:
    motor2_control = MC_ENCODER;
    motor2_target_rpm = masterPacket.get_32(2);
    break;

  case SLAVE_CMD_SET_MOTOR_1_DIR:
    temp = masterPacket.get_8(2);
    if (temp & 1)
      motor1_dir = MC_FORWARD;
    else
      motor1_dir = MC_REVERSE;    
    break;

  case SLAVE_CMD_SET_MOTOR_2_DIR:
    temp = masterPacket.get_8(2);
    if (temp & 1)
      motor2_dir = MC_FORWARD;
    else
      motor2_dir = MC_REVERSE;    
    break;

  case SLAVE_CMD_TOGGLE_MOTOR_1:
    temp = masterPacket.get_8(2);
    if (temp & 2)
      motor1_dir = MC_FORWARD;
    else
      motor1_dir = MC_REVERSE;

    if (temp & 1)
    {
      enable_motor_1();
      
      //if we interrupted a reversal, wait for the motor to get back to its old position.
      if (motor1_reversal_count > 0)
      	delay(motor1_reversal_count);
      motor1_reversal_count = 0;
    }
    else
      disable_motor_1();
    break;

  //WORKING
  case SLAVE_CMD_TOGGLE_MOTOR_2:
    temp = masterPacket.get_8(2);
    if (temp & 2)
      motor2_dir = MC_FORWARD;
    else
      motor2_dir = MC_REVERSE;

    //TODO: check to see if we're not in stepper mode.
    if (temp & 1)
      enable_motor_2();
    else
      disable_motor_2();
    break;

  //WORKING
  case SLAVE_CMD_TOGGLE_FAN:
    temp = masterPacket.get_8(2);
    if (temp & 1)
      enable_fan();
    else
      disable_fan();
    break;

#ifndef HAS_HEATED_BUILD_PLATFORM
//WORKING
  case SLAVE_CMD_TOGGLE_VALVE:
    temp = masterPacket.get_8(2);
    if (temp & 1)
      open_valve();
    else
      close_valve();
    break;
#endif

  //WORKING
  case SLAVE_CMD_SET_SERVO_1_POS:
    servo1.attach(9);
    servo1.write(masterPacket.get_8(2));
    break;

  //WORKING
  case SLAVE_CMD_SET_SERVO_2_POS:
    servo2.attach(10);
    servo2.write(masterPacket.get_8(2));
    break;

  //WORKING
  case SLAVE_CMD_FILAMENT_STATUS:
    //TODO: figure out how to detect this.
    masterPacket.add_8(255);
    break;

  //WORKING
  case SLAVE_CMD_GET_MOTOR_1_PWM:
    masterPacket.add_8(motor1_pwm);
    break;

  //WORKING
  case SLAVE_CMD_GET_MOTOR_2_PWM:
    masterPacket.add_8(motor2_pwm);
    break;

  //NEEDS TESTING
  case SLAVE_CMD_GET_MOTOR_1_RPM:
    masterPacket.add_32(motor1_current_rpm);
    break;

  //NEEDS TESTING
  case SLAVE_CMD_GET_MOTOR_2_RPM:
    masterPacket.add_32(motor1_current_rpm);
    break;

  //WORKING
  case SLAVE_CMD_SELECT_TOOL:
    //do we do anything?
    break;

  //WORKING
  case SLAVE_CMD_IS_TOOL_READY:
    masterPacket.add_8(is_tool_ready());
    break;

  case SLAVE_CMD_ABORT:
    initialize();
    break;
  case SLAVE_CMD_READ_FROM_EEPROM:
    {
      const uint16_t offset = masterPacket.get_16(2);
      const uint8_t count = masterPacket.get_8(4);
      if (count > 16) {
	masterPacket.overflow();
      } else {
	for (uint8_t i = 0; i < count; i++) {
	  masterPacket.add_8(EEPROM.read(offset+i));
	}
      }
    }
    break;
  case SLAVE_CMD_WRITE_TO_EEPROM:
    {
      uint16_t offset = masterPacket.get_16(2);
      uint8_t count = masterPacket.get_8(4);
      if (count > 16) {
	masterPacket.overflow();
      } else {
	for (uint8_t i = 0; i < count; i++) {
	  EEPROM.write(offset+i,masterPacket.get_8(5+i));
	}
	masterPacket.add_8(count);
      }
    }
    break;
  }
}
