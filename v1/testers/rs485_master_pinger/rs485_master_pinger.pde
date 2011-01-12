
#define RX_ENABLE_PIN   13
#define TX_ENABLE_PIN   12
#define DEBUG_PIN 0

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  //always enable RX.
  digitalWrite(RX_ENABLE_PIN, LOW);

  Serial.begin(38400);
  Serial1.begin(38400);

  transmit_mode();
}

void loop()
{
  for (byte i=32; i<128; i++)
  {
    digitalWrite(DEBUG_PIN, HIGH);
    delay(100);
    digitalWrite(DEBUG_PIN, LOW);

    Serial.print("OUT: ");
    print_byte(i);

    rs485_tx(i);

    delay(900);

    //check for response
    while (Serial1.available())
    {
      byte b = Serial1.read(); 
      Serial.print("IN: ");
      print_byte(i);
    }
  }
}

void print_byte(byte b)
{
  Serial.print(b, BYTE);
  Serial.print(" / ");
  Serial.print(b, DEC);
  Serial.print(" / ");
  Serial.print(b, HEX);
  Serial.print(" / ");
  Serial.print(b, BIN);
  Serial.println(".");
}

void rs485_tx(byte b)
{
  transmit_mode();
  Serial1.print(b, BYTE);

  int tmp = Serial1.read();
  while (tmp != b)
    tmp = Serial1.read();

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
