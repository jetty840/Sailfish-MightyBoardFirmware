#include "LiquidCrystal.hh"

class InterfaceBoard;

/**
 * Monitors the input state of a bunch of buttons, in polling mode.
 **/
class ButtonArray {
private:
	uint8_t previousL;
	uint8_t previousC;

public:
	ButtonArray();

	// Returns true if any of the button states have changed.
	bool scanButtons(InterfaceBoard& board);

	enum ButtonName {
		ZERO		= 1,
		ZMINUS		= 2,
		ZPLUS		= 3,
		YMINUS		= 4,
		YPLUS		= 5,
		XMINUS		= 6,
		XPLUS		= 7,
		CANCEL		= 11,
		OK			= 12,
	};
};


class InterfaceBoard {
private:
	LiquidCrystal lcd;
	ButtonArray buttons;

public:
	InterfaceBoard();

	void doInterrupt();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};
