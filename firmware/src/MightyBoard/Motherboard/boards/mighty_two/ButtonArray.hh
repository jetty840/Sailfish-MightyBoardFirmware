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

#define RESET_MASK  0x06

class ButtonArray {
private:
        uint8_t buttonPress;
        bool buttonPressWaiting;
        Timeout buttonTimeout;
        Timeout centerHold;
        Timeout rightHold;

public:
        /// Representation of the different buttons available on the keypad
        enum ButtonName {
                CENTER          = 2,
                RIGHT           = 3,
                LEFT            = 6,
                DOWN            = 4,
                UP              = 5,
                RESET			= 1,
                EGG				= 0
        };
        uint32_t ButtonDelay;
        const static uint32_t FastDelay = 100000;
		const static uint32_t SlowDelay = 500000;
        const static uint32_t ResetDelay = 10000000;
        
        void init();

        // Returns true if any of the button states have changed.
        void scanButtons();

        bool getButton(ButtonName& button);
        
        void clearButtonPress();
        void setButtonDelay(uint32_t delay);
};


#endif // BUTTONARRAY_HH
