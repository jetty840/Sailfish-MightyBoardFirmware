#define RX_ENABLE_PIN 4
#define TX_ENABLE_PIN 16
#define DEBUG_PIN 13

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  digitalWrite(DEBUG_PIN, LOW); //disable led

  digitalWrite(RX_ENABLE_PIN, LOW); //always listen.

  Serial.begin(38400);

}

void loop()
{ 
  if (Serial.available())
  {
    byte i = Serial.read();

	rs485_tx(i);

    digitalWrite(DEBUG_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(DEBUG_PIN, LOW);
  }
}

void rs485_tx(byte b)
{
  transmit_mode();
  Serial.print(b, BYTE);

  int tmp = Serial.read();
  while (tmp != b)
    tmp = Serial.read();

  receive_mode(); 
}

void transmit_mode()
{
  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx
}

void receive_mode()
{
  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx
}
