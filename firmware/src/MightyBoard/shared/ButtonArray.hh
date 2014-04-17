#ifndef BUTTONARRAY_HH
#define BUTTONARRAY_HH

#include <util/atomic.h>
#include "Types.hh"
#include "Timeout.hh"

// TODO: Make this an interface?

/// The button array modules manages an array of buttons, and maintains
/// a buffer of the last pressed button. It has two entry points: a fast
/// #scanButtons, which is a fast button scanning routine that should be
/// called from an interrupt, and #getButton, which should be called by a
/// slow loop that has time to respond to the button.
///
/// Porting Notes:
/// This modules uses low-level port registers, and must be re-written for
/// each board archetecture. This should be done by adding a ButtonArray.cc
/// definition in the board directory.
/// \ingroup HardwareLibraries

// This is an abstract class for the ButtonArray.  Different hardware should
// be implemented in classes that adhere to this protocol

#define CONTINUOUS_BUTTONS_MASK	0b00011111

// These bitmasks must not be within the CONTINUOUS_BUTTONS_MASK
#define IS_STICKY_MASK 		_BV(6)
#define IS_CANCEL_SCREEN_MASK	_BV(7)

//#define RESET_MASK 0x06

class ButtonArray {
protected:
  uint8_t buttonPress;
  bool buttonPressWaiting;
  Timeout buttonTimeout;

public:
  enum ButtonName {
    CENTER = 0,
    RIGHT = 1,
    LEFT = 2,
    DOWN = 3,
    UP = 4,
    NO_BUTTON = 255,
  };

  const static micros_t ContinuousButtonRepeatDelay = 320000;	//Must be larger than ButtonDelay
  const static uint32_t FastDelay                   = 100000;
  const static uint32_t SlowDelay                   = 300000;

  virtual void init() = 0;

  // Returns true if any of the button states have changed.
  virtual void scanButtons() = 0;

  virtual bool getButton(ButtonName &button) = 0;

  virtual void clearButtonPress() = 0;

  virtual bool isButtonPressed(ButtonArray::ButtonName button) = 0;

  virtual void setButtonDelay(micros_t delay) = 0;
};

#endif // BUTTONARRAY_HH
