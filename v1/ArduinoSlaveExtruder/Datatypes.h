#ifndef DATATYPES_H
#define DATATYPES_H

// motor control states.
typedef enum {
  MC_PWM = 0,
  MC_ENCODER = 1,
  MC_STEPPER = 2
} 
MotorControlStyle;

typedef enum {
  MC_FORWARD = 0,
  MC_REVERSE = 1
}
MotorControlDirection;

#endif // DATATYPES_H
