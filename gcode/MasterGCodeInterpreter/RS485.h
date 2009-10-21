unsigned long rs485_tx_count = 0;
unsigned long rs485_rx_count = 0;
unsigned long rs485_packet_count = 0;
unsigned long rs485_loopback_fails = 0;
unsigned long slave_crc_errors = 0;
unsigned long slave_timeouts = 0;

unsigned long serial_tx_count = 0;
unsigned long serial_rx_count = 0;
unsigned long serial_packet_count = 0;

void rs485_tx(byte b)
{
  rs485_tx_count++;

  Serial1.print(b, BYTE);

  //read for our own byte.
  long now = millis();
  long end = now + 10;
  int tmp = Serial1.read();
  while (tmp != b)
  {
    tmp = Serial1.read();
    if (millis() > end)
    {
      rs485_loopback_fails++;
#ifdef ENABLE_COMMS_DEBUG
      Serial.println("Loopback Fail");
#endif
      break;		
    }
  }
}

void serial_tx(byte b)
{
  serial_tx_count++;

  Serial.print(b, BYTE);
}
