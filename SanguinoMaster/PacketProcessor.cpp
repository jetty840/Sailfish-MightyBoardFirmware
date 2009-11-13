#include "Configuration.h"
#include <EEPROM.h>
#include "SDSupport.h"
#include "Tools.h"
#include <avr/wdt.h>
#include "RS485.h"
#include "WProgram.h"
#include "Commands.h"
#include "Variables.h"
#include "Version.h"
#include "Steppers.h"

// Hack until we completely kill PDEs: prototypes for methods in SanguinoMaster.pde
void initialize();
void abort_print();

SimplePacket hostPacket(serial_tx);

//buffer for our commands
uint8_t underlyingBuffer[COMMAND_BUFFER_SIZE];
CircularBuffer commandBuffer(COMMAND_BUFFER_SIZE, underlyingBuffer);

unsigned long finishedCommands;

//initialize the firmware to default state.
void init_commands()
{
  finishedCommands = 0;
  commandBuffer.clear();

}

void handle_query(byte cmd);

//handle our packets.
void process_host_packets()
{
  unsigned long start = millis();
  unsigned long end = start + PACKET_TIMEOUT;

#ifdef ENABLE_COMMS_DEBUG
    //Serial.print("IN: ");
#endif

  //do we have a finished packet?
  while (!hostPacket.isFinished())
  {
    if (Serial.available() > 0)
    {
      //digitalWrite(DEBUG_PIN, HIGH);

      //grab a byte and process it.
      byte d;
      d = Serial.read();
      hostPacket.process_byte(d);

#ifdef ENABLE_COMMS_DEBUG
      //Serial.print(d, HEX);
      //Serial.print(" ");
#endif
      serial_rx_count++;

      //keep us goign while we have data coming in.
      start = millis();
      end = start + PACKET_TIMEOUT;

      if (hostPacket.getResponseCode() == RC_CRC_MISMATCH)
      {
        //host_crc_errors++;
	digitalWrite(DEBUG_PIN, HIGH);

#ifdef ENABLE_COMMS_DEBUG
        Serial.println("Host CRC Mismatch");
#endif
      }
    }

    //are we sure we wanna break mid-packet?
    //have we timed out?
    if (millis() >= end)
    {
#ifdef ENABLE_COMMS_DEBUG
      Serial.println("Host timeout");
#endif
      break;  
    }
  }

  if (hostPacket.isFinished())
  {
    serial_packet_count++;

    byte b = hostPacket.get_8(0);
    // top bit high == bufferable command packet (eg. #128-255)
    if (b & 0x80)
    {
      if (is_capturing()) {
	// Capture this to the SD card
	capture_packet(hostPacket);
      } else {
	//do we have room?
	if (commandBuffer.remainingCapacity() >= hostPacket.getLength()) {
	  //okay, throw it in the buffer.
	  for (byte i=0; i<hostPacket.getLength(); i++)
	    commandBuffer.append(hostPacket.get_8(i));
	} else {
	  // Otherwise, we go on with an overflow packet.
	  hostPacket.overflow();
	}
      }
    }
    // top bit low == reply needed query packet (eg. #0-127)
    else
      handle_query(b);
  }

  //okay, send our response
  hostPacket.sendReply();
}

//this is for handling query commands that need a response.
void handle_query(byte cmd)
{
  //which one did we get?
  switch (cmd)
  {
    //WORKS
    case HOST_CMD_VERSION:
      //get our host version
      host_version = hostPacket.get_16(1);

      //send our version back.
      hostPacket.add_16(FIRMWARE_VERSION);
      break;

    //WORKS
    case HOST_CMD_INIT:
      //just initialize
      initialize();
      break;

    case HOST_CMD_GET_BUFFER_SIZE:
      //send our remaining buffer size.
      hostPacket.add_16(commandBuffer.remainingCapacity());
      break;

    case HOST_CMD_CLEAR_BUFFER:
      //just clear it.
      commandBuffer.clear();
      break;

    case HOST_CMD_GET_POSITION:
      //send our position
      {
	const LongPoint& c = get_position();
	hostPacket.add_32(c.x);
	hostPacket.add_32(c.y);
	hostPacket.add_32(c.z);
      }
      hostPacket.add_8(get_endstop_states());
      break;

    case HOST_CMD_GET_RANGE:
      //get our range
      {
	const LongPoint& range = get_range();
	hostPacket.add_32(range.x);
	hostPacket.add_32(range.y);
	hostPacket.add_32(range.z);
      }
      break;

    case HOST_CMD_SET_RANGE:
      //set our range to what the host tells us
      set_range(LongPoint(
			  (long)hostPacket.get_32(1),
			  (long)hostPacket.get_32(5),
			  (long)hostPacket.get_32(9)));
      break;

    case HOST_CMD_ABORT:
      //support a microcontrollers right to choice.
      abort_print();
      break;

    case HOST_CMD_PAUSE:
      if (is_machine_paused)
      {
        //unpause our machine.
        is_machine_paused = false;

        //unpause our tools
        set_tool_pause_state(false);

        //resume stepping.
	resume_stepping();
      }
      else
      {
        //pause our activity.
        is_machine_paused = true;

        //pause our tools
        set_tool_pause_state(true);

        //pause stepping
	pause_stepping();
      }
      break;

    case HOST_CMD_PROBE:
      //we dont support this yet.
      hostPacket.unsupported();
      break;

    //WORKS
    case HOST_CMD_TOOL_QUERY:
      send_tool_query(hostPacket);
      break;

  case HOST_CMD_IS_FINISHED:
    {
      bool finished = is_point_buffer_empty() && at_target() && commandBuffer.size() == 0;
      hostPacket.add_8(finished?1:0);
      break;
    }

  case HOST_CMD_READ_EEPROM:
    {
      const uint16_t offset = hostPacket.get_16(1);
      const uint8_t count = hostPacket.get_8(3);
      if (count > 16) {
	hostPacket.overflow();
      } else {
	for (uint8_t i = 0; i < count; i++) {
	  hostPacket.add_8(EEPROM.read(offset+i));
	}
      }
    }
    break;
  case HOST_CMD_WRITE_EEPROM:
    {
      uint16_t offset = hostPacket.get_16(1);
      uint8_t count = hostPacket.get_8(3);
      if (count > 16) {
	hostPacket.overflow();
      } else {
	for (uint8_t i = 0; i < count; i++) {
	  EEPROM.write(offset+i,hostPacket.get_8(4+i));
	}
	hostPacket.add_8(count);
      }
    }
    break;
  case HOST_CMD_CAPTURE_TO_FILE:
    {
      char filename[17];
      int i = 0;
      while (i < 16) {
	uint8_t c = hostPacket.get_8(i+1);
	if (c == 0) break;
	filename[i++] = c;
      }
      filename[i] = 0;
      hostPacket.add_8(start_capture(filename));
    }
    break;
  case HOST_CMD_END_CAPTURE:
    hostPacket.add_32(finish_capture());
    break;
  case HOST_CMD_PLAYBACK_CAPTURE:
    {
      char filename[17];
      int i = 0;
      while (i < 16) {
	uint8_t c = hostPacket.get_8(i+1);
	if (c == 0) break;
	filename[i++] = c;
      }
      filename[i] = 0;
      hostPacket.add_8(start_playback(filename));
    }
    break;
  case HOST_CMD_RESET:
    {
      wdt_enable(WDTO_500MS);
    }
    break;
  case HOST_CMD_NEXT_FILENAME:
    {
      uint8_t resetFlag = hostPacket.get_8(1);
      uint8_t rspCode = 0;
      if (resetFlag != 0) {
	rspCode = sd_scan_reset();
	if (rspCode != 0) {
	  hostPacket.add_8(rspCode);
	  hostPacket.add_8(0);
	  break;
	}
      }
      char fnbuf[16];
      rspCode = sd_scan_next(fnbuf,16);
      hostPacket.add_8(rspCode);
      uint8_t idx;
      for (idx = 0; (idx < 16) && (fnbuf[idx] != 0); idx++) {
	hostPacket.add_8(fnbuf[idx]);
      }
      hostPacket.add_8(0);
    }
    break;
  case HOST_CMD_GET_DBG_REG:
    {
      uint8_t regIdx = hostPacket.get_8(1);
      uint8_t rsp = 0;
      if (regIdx < MAX_DEBUG_REGISTER) {
	rsp = debugRegisters[regIdx];
      }
      hostPacket.add_8(rsp);
    }
  default:
      hostPacket.unsupported();
  }
}

//this is for handling buffered commands with no response
void handle_commands()
{
  byte flags = 0;
  
  long x;
  long y;
  long z;
  unsigned long step_delay;
  byte cmd;

  if (is_playing()) {
    while (commandBuffer.remainingCapacity() > 0 && playback_has_next()) {
      commandBuffer.append(playback_next());
    }
  } else {
    digitalWrite(DEBUG_PIN,LOW);
  }


  //do we have any commands?
  if (commandBuffer.size() > 0)
  {
    CircularBuffer::Cursor cursor = commandBuffer.newCursor();
    
    uint16_t index = 0;

    cmd = cursor.read_8();

    // Do it later if it's a point queueing command and we don't have room yet.
    if (cmd == HOST_CMD_QUEUE_POINT_ABS && 
	!point_buffer_has_room(POINT_SIZE)) {
      return;
    }

    switch(cmd)
    {
      case HOST_CMD_QUEUE_POINT_ABS:
        x = (long)cursor.read_32();
        y = (long)cursor.read_32();
        z = (long)cursor.read_32();
        step_delay = (unsigned long)cursor.read_32();
          
        queue_absolute_point(x, y, z, step_delay);
      
        break;

      case HOST_CMD_SET_POSITION:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }
	cli();
	set_position(LongPoint((long)cursor.read_32(),(long)cursor.read_32(),(long)cursor.read_32()));
	sei();
        break;

      case HOST_CMD_FIND_AXES_MINIMUM:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }

        //which ones are we going to?
        flags = cursor.read_8();

        //find them!
        seek_minimums(
          flags & 1,
          flags & 2,
          flags & 4,
          cursor.read_32(),
          cursor.read_16());

        break;

      case HOST_CMD_FIND_AXES_MAXIMUM:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }

        //find them!
        seek_maximums(
          flags & 1,
          flags & 2,
          flags & 4,
          cursor.read_32(),
          cursor.read_16());

        break;

      case HOST_CMD_DELAY:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }

        //take it easy.
        delay(cursor.read_32());
        break;

      case HOST_CMD_CHANGE_TOOL:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }

        //extruder, i choose you!
        select_tool(cursor.read_8());
        break;

      case HOST_CMD_WAIT_FOR_TOOL:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }

        //get your temp in gear, you lazy bum.
        
        //what tool / timeout /etc?
	{
	  uint8_t currentToolIndex = cursor.read_8();
	  uint16_t toolPingDelay = (uint16_t)cursor.read_16();
	  uint16_t toolTimeout = (uint16_t)cursor.read_16();
	  
	  //check to see if its ready now
	  if (!is_tool_ready(currentToolIndex))
	    {
	      //how often to ping?
	      toolNextPing = millis() + toolPingDelay;
	      toolTimeoutEnd = millis() + (toolTimeout * 1000);
	      
	      //okay, put us in ping-tool-until-ready mode
	      commandMode = COMMAND_MODE_WAIT_FOR_TOOL;
	    }
	}
        break;

      case HOST_CMD_TOOL_COMMAND:
	// Belay until we're at a good location.
	if (!is_point_buffer_empty()) { return; }
        
        send_tool_command(cursor);
        break;
    case HOST_CMD_ENABLE_AXES:
      // Belay until we're at a good location.
      if (!is_point_buffer_empty()) { return; }
      {
	unsigned char param = cursor.read_8();
	bool x = (param & 0x01) != 0;
	bool y = (param & 0x02) != 0;
	bool z = (param & 0x04) != 0;
	if ((param & 0x80) != 0) {
	  // enable axes
	  enable_steppers(x,y,z);
	} else {
	  // disable axes
	  disable_steppers(x,y,z);
	}
      }
      break;
    default:
      digitalWrite(DEBUG_PIN,HIGH);
      hostPacket.unsupported();
    }
    cursor.commit();
  }
}

void clear_command_buffer()
{
  commandBuffer.clear();
}
