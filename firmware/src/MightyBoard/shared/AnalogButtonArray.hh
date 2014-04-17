#ifndef ANALOGBUTTONARRAY_HH
#define ANALOGBUTTONARRAY_HH

#include "ButtonArray.hh"
#include <util/atomic.h>
#include "Types.hh"
#include "Timeout.hh"

#define DEBOUNCE_COUNT 2

class AnalogButtonArray : public ButtonArray {
private:
  bool adcValid;
  int16_t adcValue;
  uint8_t previousButton;
  uint8_t buttonCount;

  uint8_t buttonFromADC(int16_t adc_value);

public:
  void init();

  // Returns true if any of the button states have changed.
  void scanButtons();

  bool getButton(ButtonArray::ButtonName &button);

  void clearButtonPress();

  bool isButtonPressed(ButtonArray::ButtonName button);

  void setButtonDelay(micros_t delay);
};

#endif // ANALOGBUTTONARRAY_HH