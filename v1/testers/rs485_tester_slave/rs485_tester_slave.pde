#define RX_ENABLE_PIN 4
#define TX_ENABLE_PIN 16
#define DEBUG_PIN 13

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx
  digitalWrite(RX_ENABLE_PIN, LOW); //enable rx
  digitalWrite(DEBUG_PIN, LOW); //disable led
  
  Serial.begin(38400);
}

void loop()
{
  if (Serial.available() > 0)
  {
    digitalWrite(DEBUG_PIN, HIGH);
    Serial.println("RS485 active.");
  }
}
