
#define RX_ENABLE_PIN   13
#define TX_ENABLE_PIN   12
#define DEBUG_PIN 0

void setup()
{
  pinMode(RX_ENABLE_PIN, OUTPUT);
  pinMode(TX_ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  Serial.begin(38400);
  Serial.println("Started");
  Serial1.begin(38400);

  digitalWrite(TX_ENABLE_PIN, LOW); //disable tx
  digitalWrite(RX_ENABLE_PIN, LOW); //enable rx
}

void loop()
{
  if (Serial1.available() > 0)
  {
    int b = Serial1.read();
    
    Serial.print("IN: ");
    Serial.print(b, BYTE);
    Serial.print(" / ");
    Serial.print(b, DEC);
    Serial.print(" / ");
    Serial.print(b, HEX);
    Serial.print(" / ");
    Serial.print(b, BIN);
    Serial.println(".");
  }
}
