#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

void clear_command_buffer();
void init_commands();
void process_host_packets();
void handle_commands();

#endif // PACKET_PROCESSOR_H
