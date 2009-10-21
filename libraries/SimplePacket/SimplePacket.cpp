#include <SimplePacket.h>
#include <avr/io.h>

SimplePacket::SimplePacket(txFuncPtr myPtr)
{
  txFunc = myPtr;
  init();
}

void SimplePacket::init()
{
  //zero out our data arrays.
  for (rx_length = 0; rx_length < MAX_PACKET_LENGTH; rx_length++)
    rx_data[rx_length] = 0;
  for (tx_length = 0; tx_length < MAX_PACKET_LENGTH; tx_length++)
    tx_data[tx_length] = 0;
  
  //init our other variables.
  state = PS_START;
  response_code = RC_OK;
  target_length = 0;
  rx_length = 0;
  rx_crc = 0;
  tx_length = 0;
  tx_crc = 0;
}

//process a byte from our packet
void SimplePacket::process_byte(uint8_t b)
{
  if (state == PS_START)  // process start byte
  {
    //cool!  its the start of a packet.
    if (b == START_BYTE)
    {
      init();
      state = PS_LEN;
    }
    else
    {
      // throw an error message?
      // nah, ignore it as garbage.
      // FIXME: Consider reporting such error in a special debug mode
    }
  }
  else if (state == PS_LEN) // process length byte
  {
    target_length = b;
    rx_length = 0;
    state = PS_PAYLOAD;
    
    if (target_length > MAX_PACKET_LENGTH)
      response_code = RC_PACKET_TOO_BIG;
  }
  else if (state == PS_PAYLOAD)  // process payload byte
  {
    //just keep reading bytes while we got them.
    if (rx_length < target_length)
    {
      //keep track of CRC.
      rx_crc = _crc_ibutton_update(rx_crc, b);
      
      //will it fit?
      if (rx_length < MAX_PACKET_LENGTH)
        rx_data[rx_length] = b;
      
      //keep track.
      rx_length++;
    }
    
    //are we done?
    if (rx_length >= target_length)
      state = PS_CRC;
  }
  else if (state == PS_CRC)  // check crc
  {
    // did the packet check out?
    if (rx_crc != b)
      response_code = RC_CRC_MISMATCH;
    
    //okay, the packet is done.
    state = PS_LAST;
  }
}

bool SimplePacket::isFinished()
{
  return (state == PS_LAST);        
}

uint8_t SimplePacket::getLength()
{
  return rx_length;
}

PacketState SimplePacket::getState()
{
  return state;
}

ResponseCode SimplePacket::getResponseCode()
{
  return response_code;
}

void SimplePacket::unsupported()
{
  response_code = RC_CMD_UNSUPPORTED;
}

void SimplePacket::overflow()
{
  response_code = RC_BUFFER_OVERFLOW;
}

void SimplePacket::sendReply()
{
  //initialize our response CRC
  tx_crc = 0;
  tx_crc = _crc_ibutton_update(tx_crc, response_code);
  
  //actually send our response.
  transmit(START_BYTE);
  transmit(tx_length+1);
  transmit(response_code);
  
  //loop through our reply packet payload and send it.
  for (uint8_t i=0; i<tx_length; i++)
  {
    transmit(tx_data[i]);
    tx_crc = _crc_ibutton_update(tx_crc, tx_data[i]);
  }
  
  //okay, send our CRC.
  transmit(tx_crc);
  
  //okay, now reset.
  init();
}

void SimplePacket::sendPacket()
{
  tx_crc = 0;
  transmit(START_BYTE);
  transmit(tx_length);
  
  //loop through our reply packet payload and send it.
  for (uint8_t i=0; i<tx_length; i++)
  {
    transmit(tx_data[i]);
    tx_crc = _crc_ibutton_update(tx_crc, tx_data[i]);
  }
  
  //okay, send our CRC.
  transmit(tx_crc);
}

void SimplePacket::transmit(uint8_t d)
{
  txFunc(d);
}

//add a four byte chunk of data to our reply
void SimplePacket::add_32(uint32_t d)
{
  add_16(d);
  add_16(d >> 16);
}

//add a two byte chunk of data to our reply
void SimplePacket::add_16(uint16_t d)
{
  add_8(d & 0xff);
  add_8(d >> 8);
}

//add a byte to our reply.
void SimplePacket::add_8(uint8_t d)
{
  //only add it if it will fit.
  if (tx_length < MAX_PACKET_LENGTH)
    tx_data[tx_length++] = d;
}

uint8_t SimplePacket::get_8(uint8_t idx)
{
  return rx_data[idx];
}

uint16_t SimplePacket::get_16(uint8_t idx)
{
  return (((int16_t)get_8(idx+1)) << 8) | get_8(idx);
}

uint32_t SimplePacket::get_32(uint8_t idx)
{
  return (((uint32_t)get_16(idx+2)) << 16) | get_16(idx);
}
