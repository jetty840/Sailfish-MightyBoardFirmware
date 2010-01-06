#include "CommandQueue.hh"

#define COMMAND_BUFFER_SIZE 256

uint8_t buffer_data[COMMAND_BUFFER_SIZE];

CircularBuffer command_buffer(COMMAND_BUFFER_SIZE, buffer_data);
