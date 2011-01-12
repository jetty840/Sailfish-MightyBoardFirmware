#define RX_ENABLE_PIN 4
#define TX_ENABLE_PIN 16
#define DEBUG_PIN 13

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx
  digitalWrite(RX_ENABLE_PIN, HIGH); //disable rx
  digitalWrite(DEBUG_PIN, LOW); //disable led

  Serial.begin(38400);
}

void loop()
{
  for (int i=32; i<127; i++)
  {
    Serial.print(i, BYTE);
    Serial.print(i, BYTE);
    Serial.print(i, BYTE);
    Serial.print(i, BYTE);

    digitalWrite(DEBUG_PIN, HIGH);
    delay(100);
    digitalWrite(DEBUG_PIN, LOW);
    delay(900);
  }
}
