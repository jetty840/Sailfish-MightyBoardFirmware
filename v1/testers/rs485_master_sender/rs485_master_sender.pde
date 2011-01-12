
#define RX_ENABLE_PIN   13
#define TX_ENABLE_PIN   12
#define DEBUG_PIN 0

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

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

    Serial1.print(i, BYTE);

    delay(900);
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

void transmit_mode()
{
  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx
  digitalWrite(RX_ENABLE_PIN, HIGH); //disable rx
}

void receive_mode()
{
  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx
  digitalWrite(RX_ENABLE_PIN, LOW); //enable rx
}
