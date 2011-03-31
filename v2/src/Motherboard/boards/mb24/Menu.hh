#ifndef MENU_HH_
#define MENU_HH_

#include "Types.hh"
#include "SDCard.hh"
#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"


class Screen {
public:
	virtual micros_t getUpdateRate();

	// Do internal updates, redraw, etc
	virtual void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	virtual void reset();

	// Get notified that a button was pressed
	virtual void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


// A menu is a collection of text that can be
class Menu: public Screen {
public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	// Make any necessary state updates
	void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	void reset();

	//
	virtual void resetState();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);

protected:
	// The currently selected item
	uint8_t itemIndex;

	// The index used to make the last draw
	uint8_t lastDrawIndex;

	// Total number of items
	uint8_t itemCount;

	// The first selectable item (set this to greater than 0 if the first item(s) are a title)
	uint8_t firstItemIndex;

	// Draw an item at the current cursor position
	virtual void drawItem(uint8_t index, LiquidCrystal& lcd);

	// The given item was selected; do something about it.
	virtual void handleSelect(uint8_t index);

	// Cancel was selected, do something about it.
	virtual void handleCancel();
};


class SplashScreen: public Screen {
public:
	micros_t getUpdateRate() {return 50L * 1000L;}

	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class JogMode: public Screen {
private:
	enum distance_t {
	  DISTANCE_SHORT,
	  DISTANCE_LONG,
	};

	distance_t jogDistance;
	bool distanceChanged;

	void jog(InterfaceBoardDefinitions::ButtonName direction);

public:
	micros_t getUpdateRate() {return 50L * 1000L;}

	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class SnakeMode: public Screen {

#define MAX_SNAKE_SIZE 20      // Maximum length our snake can grow to
#define APPLES_BEFORE_GROW 4   // Number of apples the snake must eat before growing
#define START_SPEED  60        // Starting speed, in screen refresh times per turn


private:
	micros_t updateRate;

	struct coord_t {
		int8_t x;
		int8_t y;
	};

	enum direction_t {
	  DIR_NORTH,
	  DIR_EAST,
	  DIR_SOUTH,
	  DIR_WEST
	};

	int snakeLength;					// Length of our snake; this grows for every x 'apples' eaten
	coord_t snakeBody[MAX_SNAKE_SIZE];	// Table of each piece of the snakes body
	bool snakeAlive;					// The state of our snake
	direction_t snakeDirection;			// The direction the snake is heading
	coord_t applePosition;				// Location of the apple
	uint8_t applesEaten;				// Number of apples that have been eaten
//	int gameSpeed = START_SPEED;		// Speed of the game (in ms per turn)


public:
	micros_t getUpdateRate() {return updateRate;}

	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class SDMenu: public Menu {
public:
	SDMenu();

	void resetState();
protected:
	uint8_t countFiles();

	sdcard::SdErrorCode getFilename(uint8_t index, char buffer[], uint8_t buffer_size);

	void drawItem(uint8_t index, LiquidCrystal& lcd);

	void handleSelect(uint8_t index);
};


class CancelBuildMenu: public Menu {
public:
	CancelBuildMenu();

	void resetState();
protected:
	void drawItem(uint8_t index, LiquidCrystal& lcd);

	void handleSelect(uint8_t index);
};


class MonitorMode: public Screen {
private:
	CancelBuildMenu cancelBuildMenu;

public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	// Refresh the display information
	void update(LiquidCrystal& lcd, bool forceRedraw);

	// Reset the menu to it's default state
	void reset();

	// Get notified that a button was pressed
	void notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button);
};


class MainMenu: public Menu {
public:
	MainMenu();
protected:
	void drawItem(uint8_t index, LiquidCrystal& lcd);

	void handleSelect(uint8_t index);
};

#endif
