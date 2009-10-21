#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <stdint.h>
#include "CircularBuffer.h"
#include <SimplePacket.h>

//initialize our tools
void init_tools();
//ask a tool if its there.
bool ping_tool(uint8_t i);
//initialize a tool to its default state.
void init_tool(uint8_t i);
//select a tool as our current tool, and let it know.
void select_tool(uint8_t tool);
void check_tool_ready_state();
//ping the tool until it tells us its ready
void wait_for_tool_ready_state(uint8_t tool, int delay_millis, int timeout_seconds);
//is our tool ready for action?
bool is_tool_ready(uint8_t tool);
void send_tool_query(SimplePacket& hostPacket);
void send_tool_command(CircularBuffer::Cursor& cursor);
void send_tool_simple_command(uint8_t tool, uint8_t command);
void abort_current_tool();
bool send_packet();
bool read_tool_response(int timeout);
void set_tool_pause_state(bool paused);

extern unsigned long toolNextPing;
extern unsigned long toolTimeoutEnd;


#endif // _TOOLS_H_
