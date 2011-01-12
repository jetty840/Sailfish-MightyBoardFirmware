
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
}

void loop()
{
  digitalWrite(TX_ENABLE_PIN, HIGH); //enable tx
  digitalWrite(RX_ENABLE_PIN, HIGH); //disable rx
  digitalWrite(DEBUG_PIN, LOW); //disable led

  Serial.println("started");
  Serial1.println("hello?");

  //re-enable rx, disable tx
  digitalWrite(RX_ENABLE_PIN, LOW);
  digitalWrite(TX_ENABLE_PIN, LOW);

  if (Serial1.available() > 0)
  {
    digitalWrite(DEBUG_PIN, HIGH);
    Serial.println("RS485 active.");
  }
  
  delay(2000);
}
