#include "WProgram.h"

void rs485_tx(byte b)
{
  Serial.print(b, BYTE);
  
  //read for our own byte.
  long now = millis();
  long end = now + 10;
  int tmp = Serial.read();
  while (tmp != b) {
    tmp = Serial.read();
    if (millis() > end)
      break;
  }
}
