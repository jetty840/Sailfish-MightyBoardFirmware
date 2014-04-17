#include "StandardButtonArray.hh"
#include "Configuration.hh"
#include "Pin.hh"
#include <util/delay.h>

static uint8_t previousJ;
static uint8_t previousG;

// Use #define for these

// static const uint8_t ARROW_BUTTON_MAP = 0x78;
// static const uint8_t CENTER_BUTTON_MAP = 0x04;

static micros_t ButtonDelay;

#define ARROW_BUTTON_MAP 0x78 // 0b01111000
#define CENTER_BUTTON_MAP 0x04

void StandardButtonArray::init() {
  previousJ = 0;
  previousG = 0;

  ButtonDelay = SlowDelay;

  // Set all of the known buttons to inputs (see above note)
  // Set all of the known buttons to inputs
  DDRJ &= ~(ARROW_BUTTON_MAP);
  PORTJ &= ~(ARROW_BUTTON_MAP);

  DDRG &= ~(CENTER_BUTTON_MAP);
  PORTG &= ~(CENTER_BUTTON_MAP);
}

void StandardButtonArray::scanButtons() {

  // Don't bother scanning if we already have a button
  if (buttonPressWaiting ||
      (buttonTimeout.isActive() && !buttonTimeout.hasElapsed()))
    return;

  uint8_t newJ = PINJ & ARROW_BUTTON_MAP;
  uint8_t newG = PING & CENTER_BUTTON_MAP;

  buttonTimeout.clear();

  // center button
  if (newG != previousG) {
    if (!(newG & (PORTG_CENTER_BUTTON_MASK))) {
      buttonPress = CENTER;
      buttonPressWaiting = true;
      buttonTimeout.start(ButtonDelay);
    }
  }

  if (newJ != previousJ) {
    uint8_t diff = (newJ ^ previousJ);
    if (!buttonPressWaiting && diff) {
      if ((diff & PORTJ_UP_BUTTON_MASK) && !(newJ & PORTJ_UP_BUTTON_MASK)) {
        buttonPress = UP;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);
      } else if ((diff & PORTJ_DOWN_BUTTON_MASK) && !(newJ & PORTJ_DOWN_BUTTON_MASK)) {
        buttonPress = DOWN;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);        
      } else if ((diff & PORTJ_RIGHT_BUTTON_MASK) && !(newJ & PORTJ_RIGHT_BUTTON_MASK)) {
        buttonPress = RIGHT;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);        
      } else if ((diff & PORTJ_LEFT_BUTTON_MASK) && !(newJ & PORTJ_LEFT_BUTTON_MASK)) {
        buttonPress = LEFT;
        buttonPressWaiting = true;
        buttonTimeout.start(ButtonDelay);        
      }
    }
  }

  previousG = newG;
  previousJ = newJ;
}

bool StandardButtonArray::getButton(ButtonName &button) {
  bool buttonValid;
  uint8_t buttonNumber;

  ATOMIC_BLOCK(ATOMIC_FORCEON) {
    buttonValid = buttonPressWaiting;
    buttonNumber = buttonPress;
    buttonPressWaiting = false;
  }

  if (buttonValid) {
    button = (ButtonName)(buttonNumber);
  }

  return buttonValid;
}

void StandardButtonArray::clearButtonPress() {

  previousJ = ARROW_BUTTON_MAP;
  previousG = CENTER_BUTTON_MAP;
}

// Returns true is button is depressed
bool
StandardButtonArray::isButtonPressed(StandardButtonArray::ButtonName button) {

  // Buttons are active low
  switch (button) {
  case NO_BUTTON:
    // Included for completeness but not tested or used.
    // If any button but == 0 then a button is being pressed
    // and isPressed(NO_BUTTON) returns false
    if ((PING & PORTG_CENTER_BUTTON_MASK) != PORTG_CENTER_BUTTON_MASK ||
        (PINJ & ARROW_BUTTON_MAP) != ARROW_BUTTON_MAP)
      return false;
  case CENTER:
    if (PING & PORTG_CENTER_BUTTON_MASK)
      return false;
    break;
  case RIGHT:
    if (PINJ & PORTJ_RIGHT_BUTTON_MASK)
      return false;
    break;
  case LEFT:
    if (PINJ & PORTJ_LEFT_BUTTON_MASK)
      return false;
    break;
  case DOWN:
    if (PINJ & PORTJ_DOWN_BUTTON_MASK)
      return false;
    break;
  case UP:
    if (PINJ & PORTJ_UP_BUTTON_MASK)
      return false;
    break;
  }

  return true;
}

void StandardButtonArray::setButtonDelay(uint32_t delay) {
  ButtonDelay = delay;
}