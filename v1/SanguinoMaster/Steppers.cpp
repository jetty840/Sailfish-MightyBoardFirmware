#include "WProgram.h"
#include "Steppers.h"
#include "CircularBuffer.h"
#include "EEPROMOffsets.h"
#include <EEPROM.h>
#include "Configuration.h"
#include "Timer1.h"
#include "Utils.h"


//our point queue variables
uint8_t rawPointBuffer[POINT_QUEUE_SIZE * POINT_SIZE];
CircularBuffer pointBuffer((POINT_QUEUE_SIZE * POINT_SIZE), rawPointBuffer);


//init our variables
volatile long max_delta;

volatile bool is_point_queue_empty = true;

inline void grab_next_point();
inline bool read_switch(int8_t pin);

/// We need a logical XOR, so we'll implement it here.
inline bool logic_xor(bool a, bool b) {
  return a?!b:b;
}

class StepperAxis {
public:
  volatile long counter;
  volatile bool direction;

private:
  bool inverted;
  const int8_t stepPin;
  const int8_t dirPin;
  const int8_t enablePin;
  const int8_t minPin;
  const int8_t maxPin;

public:
  volatile long currentSteps;
  volatile long targetSteps;
  volatile long deltaSteps;
  volatile long rangeSteps;

private:
  // Seeking data
  bool seeking; //< seeking endstop?
  bool seekPositive; //< seeking in positive direction?
  bool seekBackoff; //< in backoff mode?

public:
  StepperAxis(int8_t step, int8_t dir, int8_t enable,
	      int8_t min, int8_t max) :
    counter(0), direction(false),
    stepPin(step), dirPin(dir), enablePin(enable),
    minPin(min), maxPin(max),
    rangeSteps(0)
  {
    zeroSteps();
  }

  void setTarget(long target) {
    targetSteps = target;
    //figure out our deltas
    deltaSteps = targetSteps - currentSteps;
    //what direction?
    setDirection(deltaSteps >= 0);
    //now get us absolute coords
    deltaSteps = abs(deltaSteps);
    //enable our steppers if needed.
    if (deltaSteps > 0) {
      enableStepper();
    }
  }

  void startSeek(bool positive) {
    seeking = true;
    seekPositive = positive;
    setDirection(seekPositive);
    enableStepper();
    seekBackoff = false;
  }

  bool isSeeking() {
    return seeking;
  }

  /// True if still seeking, false if done.
  /// The "doBackoff" parameter indicates that this is a backoff step.  (Backoff should be
  /// much slower that the forward motion.)
  bool seekStep(bool doBackoff) {
    if (!seeking) return false;
    bool triggered = seekPositive?read_switch(maxPin):read_switch(minPin);
    if (!seekBackoff) {
      doStep();
      if (triggered) {
	seekBackoff = true;
	// reverse direction
	setDirection(!seekPositive);
      }
    } else {
      if (doBackoff) {
	doStep();
      }
      if (!triggered) {
	seekBackoff = false;
	seeking = false;
	if (seekPositive) {
	  rangeSteps = currentSteps;
	} else {
	  currentSteps = 0;
	}
	enableStepper(false);
	return false;
      }
    }
    return true;
  }

    
  void zeroSteps() {
    currentSteps = targetSteps = deltaSteps = 0;
  }

  void setInverted(bool invert) {
    inverted = invert;
  }

  void setDirection(bool positive) {
    direction = positive;
    digitalWrite(dirPin, logic_xor(inverted,direction));
  }

  void doStep() {
    if (direction) currentSteps++;
    else currentSteps--;
    digitalWrite(stepPin, HIGH);
#ifdef STEP_DELAY
    delayMicrosecondsInterruptible(STEP_DELAY);
#endif
    digitalWrite(stepPin, LOW);
  }

  void initialize() {
    //initialize all our pins.
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(enablePin, OUTPUT);
    pinMode(minPin, INPUT);
    pinMode(maxPin, INPUT);
#if SENSORS_INVERTING == 1
    // If we are using inverting endstops, we'll turn on the pull-ups on these pins.
    // This enables us to operate without endstops if necessary.
    digitalWrite(minPin, HIGH);
    digitalWrite(maxPin, HIGH);
#endif
  }

  void enableStepper(bool enable = true)
  {
    digitalWrite(enablePin, enable?STEPPER_ENABLE:STEPPER_DISABLE);
  }

  void doInterrupt() {
    // check endstop
    if ( (direction && read_switch(maxPin)) ||
	 (!direction && read_switch(minPin)) ) {
      enableStepper(false); 
      return;
    }
    //increment our x counter, and take steps if required.
    if (currentSteps != targetSteps) {
      counter += deltaSteps;
      if (counter > 0) {
	doStep();
	counter -= max_delta;
      }
    }
  }
  
  bool atTarget() {
    return currentSteps == targetSteps;
  }

};

#define AXIS_COUNT 3
StepperAxis axes[AXIS_COUNT] = {
  StepperAxis(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN, X_MIN_PIN, X_MAX_PIN),
  StepperAxis(Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_MIN_PIN, Y_MAX_PIN),
  StepperAxis(Z_STEP_PIN, Z_DIR_PIN, Z_ENABLE_PIN, Z_MIN_PIN, Z_MAX_PIN)
};


//initialize our stepper drivers
void init_steppers()
{
  //clear our point buffer
  pointBuffer.clear();

  //initialize all our pins.
  for (int i = 0; i < AXIS_COUNT; i++) {
    axes[i].initialize();
  }

  // Load the inversion data if it's available.
  if (hasEEPROMSettings()) {
    uint8_t inversions = EEPROM.read(EEPROM_AXIS_INVERSION_OFFSET);
    for (int i = 0; i < AXIS_COUNT; i++) {
      axes[i].setInverted((inversions & (0x01 << i)) != 0);
    }
  } 

  //turn them off to start.
  disable_steppers();

  //zero our deltas.
  for (int i = 0; i < AXIS_COUNT; i++) {
    axes[i].zeroSteps();
  }
    
  //prep timer 1 for handling DDA stuff.
  setupTimer1Interrupt();
  setTimer1Micros(2500);
  enableTimer1Interrupt();
}

void seek_dir(bool seek[], bool positive, unsigned long step_delay, unsigned int timeout_seconds);

void seek_minimums(bool find_x, bool find_y, bool find_z, unsigned long step_delay, unsigned int timeout_seconds) {
  bool seek[AXIS_COUNT] = { find_x, find_y, find_z };
  seek_dir(seek, false, step_delay, timeout_seconds);
}

void seek_maximums(bool find_x, bool find_y, bool find_z, unsigned long step_delay, unsigned int timeout_seconds) {
  bool seek[AXIS_COUNT] = { find_x, find_y, find_z };
  seek_dir(seek, true, step_delay, timeout_seconds);
}


void seek_dir(bool seek[], bool positive, unsigned long step_delay, unsigned int timeout_seconds)
{
  unsigned long start = millis();
  unsigned long end = millis() + (timeout_seconds*1000);

  //no dda interrupts.
  disableTimer1Interrupt();

  for (int i = 0; i < AXIS_COUNT; i++) {
    if (seek[i]) { axes[i].startSeek(positive); }
  }

  // backoff steps: every 100ms.
  const unsigned long backoffIntervalMillis = 100L;
  long msToBack = backoffIntervalMillis * 1000L;

  //do it until we time out.
  while (millis() < end)
  {
    boolean backoff = false;
    if (msToBack <= 0) {
      msToBack = backoffIntervalMillis*1000L;
      backoff = true;
    } else {
      msToBack -= step_delay;
    }

    boolean cont = false;
    for (int i = 0; i < AXIS_COUNT; i++) {
      cont = cont || axes[i].seekStep(backoff);
    }
    if (!cont) break;

    //do our delay for our axes.
    if (step_delay <= 65535)
      delayMicrosecondsInterruptible(step_delay);
    else
      delay(step_delay/1000);

  }

  //turn on point seeking agian.
  enableTimer1Interrupt();
}


inline void grab_next_point()
{
  //can we even step to this?
  if (pointBuffer.size() >= POINT_SIZE)
  {
    max_delta = 0;
    for (int i = 0; i < AXIS_COUNT; i++) {
      //whats our target?
      axes[i].setTarget((long)pointBuffer.remove_32());
      max_delta = max(max_delta,axes[i].deltaSteps);
    }

    for (int i = 0; i < AXIS_COUNT; i++) {
      axes[i].counter = -max_delta/2;
    }

    // explicitly shut down Z axis stepper if not required
    if (axes[2].deltaSteps == 0) { axes[2].enableStepper(false); }

    //start the move!
    setTimer1Micros(pointBuffer.remove_32());
    enableTimer1Interrupt();
  }
  else
    is_point_queue_empty = true; //only real place to check.  
}

//do a single step on our DDA line!
SIGNAL(SIG_OUTPUT_COMPARE1A)
{
  for (int i =0; i < AXIS_COUNT; i++) {
    axes[i].doInterrupt();
  }

  //we're either at our target
  if (at_target())
  {
//    finishedPoints++;
//    Serial.print("Finished:");
//    Serial.println(finishedPoints, DEC);
    grab_next_point();
  }
}


//figure out if we're at a switch or not
inline bool read_switch(int8_t pin)
{
  //dual read as crude debounce
  if (SENSORS_INVERTING)
    return !digitalRead(pin) && !digitalRead(pin);
  else
    return digitalRead(pin) && digitalRead(pin);
}

// enable our steppers so we can move them.  disable any steppers
// not about to be set in motion to reduce power and heat.
// TODO: make this a configuration option (HOLD_AXIS?); there are some
// situations (milling) where you want to leave the steppers on to
// hold the position.
// ZMS: made X/Y axes always on once used.
void enable_needed_steppers()
{
  for (int i = 0; i < AXIS_COUNT; i++) {
    if ( axes[i].deltaSteps > 0 ) { axes[i].enableStepper(); }
  }
}

void enable_steppers(bool x, bool y, bool z)
{
  if (x) axes[0].enableStepper();
  if (y) axes[1].enableStepper();
  if (z) axes[2].enableStepper();
}

void disable_steppers(bool x, bool y, bool z)
{
  //disable our steppers
  if (x) axes[0].enableStepper(false);
  if (y) axes[1].enableStepper(false);
  if (z) axes[2].enableStepper(false);
}

//turn off steppers to save juice / keep things cool.
void disable_steppers()
{
  //disable our steppers
  disable_steppers(true,true,true);
}

//read all of our states into a single byte.
int8_t get_endstop_states()
{
  int8_t state = 0;

  //each one is its own bit in the byte.
  state |= read_switch(Z_MAX_PIN) << 5;
  state |= read_switch(Z_MIN_PIN) << 4;
  state |= read_switch(Y_MAX_PIN) << 3;
  state |= read_switch(Y_MIN_PIN) << 2;
  state |= read_switch(X_MAX_PIN) << 1;
  state |= read_switch(X_MIN_PIN);

  return state;
}

//queue a point for us to move to
void queue_absolute_point(long x, long y, long z, unsigned long micros)
{
  //wait until we have free space
  while (pointBuffer.remainingCapacity() < POINT_SIZE)
    delay(1);

  disableTimer1Interrupt();

  //okay, add in our points.
  pointBuffer.append_32(x);
  pointBuffer.append_32(y);
  pointBuffer.append_32(z);
  pointBuffer.append_32(micros);

  //just in case we got interrupted and it changed.
  is_point_queue_empty = false;

  enableTimer1Interrupt();
}

bool is_point_buffer_empty()
{
  //okay, we got points in the buffer.
  if (!is_point_queue_empty)
    return false;

  //still working on a point.
  //todo: do we need this?
  if (!at_target())
    return false;

  //nope, we're done.
  return true;
}

bool at_target()
{
  for (int i =0; i < AXIS_COUNT; i++) {
    if (!axes[i].atTarget()) { return false; }
  }
  return true;
}

void wait_until_target_reached()
{
  //todo: check to see if this is what is locking up our code?
  while(!is_point_buffer_empty())
    delay(1);
}

bool point_buffer_has_room(uint8_t size)
{
  return (pointBuffer.remainingCapacity() >= size);
}


void set_position(const LongPoint& pos)
{
  axes[0].targetSteps = axes[0].currentSteps = pos.x;
  axes[1].targetSteps = axes[1].currentSteps = pos.y;
  axes[2].targetSteps = axes[2].currentSteps = pos.z;
}

const LongPoint get_position()
{
  LongPoint p;
  p.x = axes[0].currentSteps;
  p.y = axes[1].currentSteps;
  p.z = axes[2].currentSteps;
  return p;
}


void set_range(const LongPoint& range)
{
  axes[0].rangeSteps = range.x;
  axes[1].rangeSteps = range.y;
  axes[2].rangeSteps = range.z;
}

const LongPoint get_range()
{
  LongPoint p;
  p.x = axes[0].rangeSteps;
  p.y = axes[1].rangeSteps;
  p.z = axes[2].rangeSteps;
  return p;
}

void resume_stepping()
{
  enable_needed_steppers();
  enableTimer1Interrupt();
}

void pause_stepping()
{
  disableTimer1Interrupt();
  disable_steppers();
}
