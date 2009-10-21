#define RX_ENABLE_PIN 4
#define TX_ENABLE_PIN 16
#define DEBUG_PIN 13

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  digitalWrite(DEBUG_PIN, LOW); //disable led

  Serial.begin(38400);
  receive_mode();
}

void loop()
{ 
  if (Serial.available())
  {
    byte i = Serial.read();

    digitalWrite(DEBUG_PIN, HIGH);
    delayMicroseconds(100);
    digitalWrite(DEBUG_PIN, LOW);
  }
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
