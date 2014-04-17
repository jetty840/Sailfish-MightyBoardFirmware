#ifndef STANDARDBUTTONARRAY_HH
#define STANDARDBUTTONARRAY_HH

#include "ButtonArray.hh"
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

#define PORTG_CENTER_BUTTON_MASK (1 << 2)
#define PORTJ_RIGHT_BUTTON_MASK (1 << 3)
#define PORTJ_DOWN_BUTTON_MASK (1 << 4)
#define PORTJ_UP_BUTTON_MASK (1 << 5)
#define PORTJ_LEFT_BUTTON_MASK (1 << 6)

class StandardButtonArray : public ButtonArray {

public:
  void init();

  // Returns true if any of the button states have changed.
  void scanButtons();

  bool getButton(ButtonName &button);

  void clearButtonPress();

  bool isButtonPressed(ButtonArray::ButtonName button);

  void setButtonDelay(micros_t delay);
};

#endif // STANDARDBUTTONARRAY_HH