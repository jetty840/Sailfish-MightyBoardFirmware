#include "PSU.h"
#include "Configuration.h"
#include "WProgram.h"

void init_psu()
{
#ifdef PS_ON_PIN
  pinMode(PS_ON_PIN, OUTPUT);
  turn_psu_on();
#endif
}

void turn_psu_on()
{
#ifdef PS_ON_PIN
  digitalWrite(PS_ON_PIN, LOW);
  delay(2000); //wait for PSU to actually turn on.
#endif
}

void turn_psu_off()
{
#ifdef PS_ON_PIN
  digitalWrite(PS_ON_PIN, HIGH);
#endif
}
