#include "Timer1.h"
#include "Variables.h"
#include "Configuration.h"

//these routines provide an easy interface for controlling timer1 interrupts

//this handles the timer interrupt event
SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  #if MOTOR_STYLE == 1
    //do encoder stuff here.
  #else
    //increment our index
    if (motor1_dir == MC_FORWARD)
      stepper_index = (stepper_index - 1) % 8;
    else
      stepper_index = (stepper_index + 1) % 8;
      
    //coil 1
    digitalWrite(MOTOR_1_DIR_PIN,   coil_a_direction & (1 << stepper_index));
    if (coil_a_enabled & (1 << stepper_index))
      analogWrite(MOTOR_1_SPEED_PIN, stepper_high_pwm);
    else
      analogWrite(MOTOR_1_SPEED_PIN, stepper_low_pwm);

    //coil 2
    digitalWrite(MOTOR_2_DIR_PIN,   coil_b_direction & (1 << stepper_index));
    if (coil_b_enabled & (1 << stepper_index))
      analogWrite(MOTOR_2_SPEED_PIN, stepper_high_pwm);
    else
      analogWrite(MOTOR_2_SPEED_PIN, stepper_low_pwm);
  #endif
}

void enableTimer1Interrupt()
{
  //enable our interrupt!
  TIMSK1 |= (1<<OCIE1A);
}

void disableTimer1Interrupt()
{
  TIMSK1 &= ~(1<<ICIE1);
  TIMSK1 &= ~(1<<OCIE1A);
}

void setTimer1Resolution(byte r)
{
  //from table 15-5 in that atmega168 datasheet:
  // we're setting CS12 - CS10 which correspond to the binary numbers 0-5
  // 0 = no timer
  // 1 = no prescaler
  // 2 = clock/8
  // 3 = clock/64
  // 4 = clock/256
  // 5 = clock/1024

  if (r > 5)
    r = 5;

  TCCR1B &= B11111000;
  TCCR1B |= r;
}

void setTimer1Ceiling(unsigned int c)
{
  OCR1A = c;
}


unsigned int getTimer1Ceiling(unsigned long ticks)
{
  // our slowest speed at our highest resolution ( (2^16-1) * 0.0625 usecs = 4095 usecs)
  if (ticks <= 65535L)
    return (ticks & 0xffff);
  // our slowest speed at our next highest resolution ( (2^16-1) * 0.5 usecs = 32767 usecs)
  else if (ticks <= 524280L)
    return ((ticks / 8) & 0xffff);
  // our slowest speed at our medium resolution ( (2^16-1) * 4 usecs = 262140 usecs)
  else if (ticks <= 4194240L)
    return ((ticks / 64) & 0xffff);
  // our slowest speed at our medium-low resolution ( (2^16-1) * 16 usecs = 1048560 usecs)
  else if (ticks <= 16776960L)
    return (ticks / 256);
  // our slowest speed at our lowest resolution ((2^16-1) * 64 usecs = 4194240 usecs)
  else if (ticks <= 67107840L)
    return (ticks / 1024);
  //its really slow... hopefully we can just get by with super slow.
  else
    return 65535;
}

byte getTimer1Resolution(unsigned long ticks)
{
  // these also represent frequency: 1000000 / ticks / 2 = frequency in hz.

  // our slowest speed at our highest resolution ( (2^16-1) * 0.0625 usecs = 4095 usecs (4 millisecond max))
  // range: 8Mhz max - 122hz min
  if (ticks <= 65535L)
    return 1;
  // our slowest speed at our next highest resolution ( (2^16-1) * 0.5 usecs = 32767 usecs (32 millisecond max))
  // range:1Mhz max - 15.26hz min
  else if (ticks <= 524280L)
    return 2;
  // our slowest speed at our medium resolution ( (2^16-1) * 4 usecs = 262140 usecs (0.26 seconds max))
  // range: 125Khz max - 1.9hz min
  else if (ticks <= 4194240L)
    return 3;
  // our slowest speed at our medium-low resolution ( (2^16-1) * 16 usecs = 1048560 usecs (1.04 seconds max))
  // range: 31.25Khz max - 0.475hz min
  else if (ticks <= 16776960L)
    return 4;
  // our slowest speed at our lowest resolution ((2^16-1) * 64 usecs = 4194240 usecs (4.19 seconds max))
  // range: 7.812Khz max - 0.119hz min
  else if (ticks <= 67107840L)
    return 5;
  //its really slow... hopefully we can just get by with super slow.
  else
    return 5;
}

void setTimer1Ticks(unsigned long ticks)
{
  // ticks is the delay between interrupts in 62.5 nanosecond ticks.
  //
  // we break it into 5 different resolutions based on the delay. 
  // then we set the resolution based on the size of the delay.
  // we also then calculate the timer ceiling required. (ie what the counter counts to)
  // the result is the timer counts up to the appropriate time and then fires an interrupt.

  setTimer1Ceiling(getTimer1Ceiling(ticks));
  setTimer1Resolution(getTimer1Resolution(ticks));
}

void setupTimer1Interrupt()
{
  //clear the registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  TIMSK1 = 0;

  //waveform generation = 0100 = CTC
  TCCR1B &= ~(1<<WGM13);
  TCCR1B |=  (1<<WGM12);
  TCCR1A &= ~(1<<WGM11); 
  TCCR1A &= ~(1<<WGM10);

  //output mode = 00 (disconnected)
  TCCR1A &= ~(1<<COM1A1); 
  TCCR1A &= ~(1<<COM1A0);
  TCCR1A &= ~(1<<COM1B1); 
  TCCR1A &= ~(1<<COM1B0);

  //start off with a slow frequency.
  setTimer1Resolution(5);
  setTimer1Ceiling(65535);
  disableTimer1Interrupt();
}
