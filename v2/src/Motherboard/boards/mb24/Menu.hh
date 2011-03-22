#ifndef MENU_HH_
#define MENU_HH_

#include "SDCard.hh"
#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"


class Screen {
public:
	// Do internal updates, redraw, etc
	virtual void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	virtual void reset();

	// Get notified that a button was pressed
	virtual void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class MonitorMode: public Screen {
private:
	int8_t pos;

public:
	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class JogMode: public Screen {
private:
	void jog(InterfaceBoardDefinitions::ButtonName direction);

public:
	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


// A menu is a collection of text that can be
class Menu: public Screen {
public:
	// Make any necessary state updates
	void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	virtual void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);

protected:
	// The currently selected item
	uint8_t itemIndex;

	// The index used to make the last draw
	uint8_t lastDrawIndex;

	// Total number of items
	uint8_t itemCount;

	// Draw an item at the current cursor position
	virtual void drawItem(uint8_t index, LiquidCrystal& lcd);

	// The given item was selected; do something about it.
	virtual void handleSelect(uint8_t index);

	// Cancel was selected, do something about it.
	virtual void handleCancel();
};


class SDMenu: public Menu {
public:
	SDMenu();

	void reset();
protected:
	uint8_t countFiles();

	sdcard::SdErrorCode getFilename(uint8_t index, char buffer[], uint8_t buffer_size);

	void drawItem(uint8_t index, LiquidCrystal& lcd);

	void handleSelect(uint8_t index);
};


class MainMenu: public Menu {
public:
	MainMenu();
protected:
	void drawItem(uint8_t index, LiquidCrystal& lcd);

	void handleSelect(uint8_t index);

	SDMenu sdMenu;
	MonitorMode monitor;
	JogMode jogger;
};




#endif
