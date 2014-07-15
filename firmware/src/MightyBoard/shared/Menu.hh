// JogModeScreen
// ProfileChangeNameModeScreen
// PauseAtZPosScreen

#ifndef MENU_HH_
#define MENU_HH_

#include "Types.hh"
#include "ButtonArray.hh"
#include "LiquidCrystalSerial.hh"
#include "Configuration.hh"
#include "CircularBuffer.hh"
#include "Timeout.hh"
#include "Host.hh"
#include "UtilityScripts.hh"
#include "EepromMap.hh"

#include "StepperAccelPlanner.hh"

extern uint8_t lastFileIndex;

enum FilamentStates{
	FILAMENT_HEATING,
	FILAMENT_WAIT,
	FILAMENT_STOP,
	FILAMENT_OK,
	FILAMENT_DONE,
	FILAMENT_EXIT,
	FILAMENT_TIMEOUT
};

enum FilamentScript{
	FILAMENT_RIGHT_FOR,
	FILAMENT_LEFT_FOR,
	FILAMENT_RIGHT_REV,
	FILAMENT_LEFT_REV,
};

#ifndef SINGLE_EXTRUDER

enum AlignmentState {
	ALIGNMENT_START,
	ALIGNMENT_PRINT,
	ALIGNMENT_EXPLAIN1,
	ALIGNMENT_EXPLAIN2,
	ALIGNMENT_SELECT,
	ALIGNMENT_END,
	ALIGNMENT_QUIT
};

#endif

/// The screen class defines a standard interface for anything that should
/// be displayed on the LCD.
class Screen {
public:
        /// Get the rate that this display should be updated. This is called
        /// after every screen display, so it can be used to dynamically
        /// adjust the update rate. This can be as fast as you like, however
        /// refreshing too fast during a build is certain to interfere with
        /// the serial and stepper processes, which will decrease build quality.
        /// \return refresh interval, in microseconds.

	/// Replacing getUpdateRate() with a public variable of type micros_t
	/// actually adds about 400 bytes to the binary!  (Most odd, but there
	/// you have it.)

	virtual micros_t getUpdateRate() = 0;

        /// Update the screen display,
        /// \param[in] lcd LCD to write to
        /// \param[in] forceRedraw if true, redraw the entire screen. If false,
        ///                        only updated sections need to be redrawn.
	virtual void update(LiquidCrystalSerial& lcd, bool forceRedraw) = 0;

        /// Reset the screen to it's default state
	virtual void reset() = 0;

        /// Get a notification that a button was pressed down.
        /// This function is called for every button that is pressed. Screen
        /// logic can be updated, however the screen should not be redrawn
        /// until update() is called again.
        ///
        /// Note that the current implementation only supports one button
        /// press at a time, and will discard any other events.
        /// \param button Button that was pressed
        virtual void notifyButtonPressed(ButtonArray::ButtonName button) = 0;

	uint8_t optionsMask;	// Bits:
				// 0 = CENTER (continuous buttons)
				// 1 = RIGHT  (continuous buttons)
				// 2 = LEFT   (continuous buttons)
				// 3 = DOWN   (continuous buttons)
				// 4 = UP     (continuous buttons)
				// 5 = unused
				// 6 = isSticky 	- when a build is finished, menus are automatically popped off the stack to
				//		 	  return to the main menu or utilities menu.
				//		 	  When isSticky is set, popping stops at this screen
				// 7 = isCancelScreen	- check if the screen is a cancel screen in case other button
        			//			  wait behavior is activated

	Screen(uint8_t oMask = (uint8_t)0): optionsMask(oMask) {}
};


/// The menu object can be used to display a list of options on the LCD
/// screen. It handles updating the display and responding to button presses
/// automatically.
class Menu: public Screen {

public:
	Menu(uint8_t optionsMask, uint8_t count): Screen(optionsMask), itemCount(count) {}

	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	virtual void resetState();

        void notifyButtonPressed(ButtonArray::ButtonName button);

protected:

        bool needsRedraw;               ///< set to true if a menu item changes out of sequence
	bool lineUpdate;	       	///< flags the menu to update the current line
        volatile uint8_t itemIndex;     ///< The currently selected item
        uint8_t lastDrawIndex;          ///< The index used to make the last draw
        uint8_t itemCount;              ///< Total number of items
        uint8_t firstItemIndex;         ///< The first selectable item. Set this
                                        ///< to greater than 0 if the first
                                        ///< item(s) are a title)

        /// Draw an item at the current cursor position.
        /// \param[in] index Index of the item to draw
        /// \param[in] LCD screen to draw onto
	virtual void drawItem(uint8_t index, LiquidCrystalSerial& lcd) = 0;

        /// Handle selection of a menu item
        /// \param[in] index Index of the menu item that was selected
	virtual void handleSelect(uint8_t index);
};

/// The Counter menu builds on menu to allow selecting number values .
class CounterMenu: public Menu {

public:
	CounterMenu(uint8_t optionsMask, uint8_t count): Menu(optionsMask, count) {}

	micros_t getUpdateRate() {return 200L * 1000L;}

	void notifyButtonPressed(ButtonArray::ButtonName button);

protected:
	bool selectMode;        ///< true if in counter change state
	uint8_t selectIndex;        ///< The currently selected item, in a counter change state
	uint8_t firstSelectIndex;   ///< first line with selectable item
	uint8_t lastSelectIndex;   ///< last line with a selectable item

	void reset();

    // must be virtual for derived classes
	virtual void drawItem(uint8_t index, LiquidCrystalSerial& lcd) { }
	virtual void handleCounterUpdate(uint8_t index, int8_t up) { }
};

/// Display a welcome splash screen, that removes itself when updated.
class SplashScreen: public Screen {

public:
	bool hold_on;

	SplashScreen() : hold_on(false) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

extern SplashScreen splashScreen;

class CancelBuildMenu: public Menu {

public:
	CancelBuildMenu();

	void resetState();

	bool isCancelScreen(){return true;}

	uint8_t state;

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};

class BuildStatsScreen: public Screen {

private:
	const static uint8_t UPDATE_COUNT_MAX = 8;
	uint8_t update_count;
#if defined(AUTO_LEVEL)
        uint8_t flip_flop;
#endif

public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

/// load / unload filament options
class FilamentScreen: public Screen {

public:
        uint8_t leaveHeatOn, checkHeatOn;

	micros_t getUpdateRate() {return 500L * 1000L;}

	void setScript(FilamentScript script);

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);

	uint8_t filamentState;

private:
	Timeout filamentTimer;
	int16_t filamentTemp[EXTRUDERS];
	uint8_t axisID, toolID;
	uint8_t digiPotOnEntry;
	uint8_t restoreAxesEnabled;
	bool forward;
	bool helpText;
	bool needsRedraw;

	void startMotor();
	void stopMotor();
};

class FilamentMenu: public Menu {

public:
        FilamentMenu();

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void resetState();
};

class PauseAtZPosScreen: public Screen {

private:
	float pauseAtZPos;
	int multiplier;

public:
	PauseAtZPosScreen() :
	     Screen(_BV((uint8_t)ButtonArray::UP) |
		    _BV((uint8_t)ButtonArray::DOWN) |
		    _BV((uint8_t)ButtonArray::RIGHT)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

#if defined(AUTO_LEVEL)

class MaxZDiffScreen: public Screen {

private:
	float fmax_zdelta;

public:
	MaxZDiffScreen() : Screen(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

#endif

class ChangeSpeedScreen: public Screen {

private:
	uint8_t alterSpeed;
	FPTYPE  speedFactor;

public:
        ChangeSpeedScreen(): Screen(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class ChangeTempScreen: public Screen {

private:
        uint8_t activeToolhead;
        uint16_t altTemp;

        void getTemp();

public:
        ChangeTempScreen() : Screen(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class ActiveBuildMenu: public Menu {

private:
	//Fan ON/OFF
	bool fanState;
	bool is_hot;
	bool is_heating;
	uint8_t is_paused;

public:
        bool filamentLoadForceHeatOff;
	ActiveBuildMenu();

	void resetState();

	micros_t getUpdateRate() {return 100L * 1000L;}

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};



/// Display a message for the user, and provide support for
/// user-specified pauses.
class MessageScreen: public Screen {

private:
	uint8_t x, y;
#define MSG_SCR_BUF_SIZE ( 1 + LCD_SCREEN_WIDTH * LCD_SCREEN_HEIGHT )
	char message[MSG_SCR_BUF_SIZE];
	uint8_t cursor;
	bool needsRedraw;
	Timeout timeout;

public:
	bool incomplete;

	MessageScreen() : needsRedraw(false) { message[0] = '\0'; }

        /// poll if the screen is waiting on a timer
	bool screenWaiting(void);

	void setXY(uint8_t xpos, uint8_t ypos) { x = xpos; y = ypos; }

	void addMessage(CircularBuffer& buf);
	void addMessage(const prog_uchar msg[]);
	void clearMessage();
	void setTimeout(uint8_t seconds);//, bool pop);
	void refreshScreen();

	micros_t getUpdateRate() {return 100L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);

	bool buttonsDisabled;
};


class JogModeScreen: public Screen {

private:
	enum distance_t {
		DISTANCE_SHORT,
		DISTANCE_LONG,
		DISTANCE_CONT,
	};
	enum jogmode_t {
		JOG_MODE_X,
		JOG_MODE_Y,
		JOG_MODE_Z
	};

	distance_t jogDistance;
        jogmode_t  jogMode;
	uint8_t    digiPotOnEntry[3];
	bool       distanceChanged, modeChanged;
	bool       jogging;

	void jog(ButtonArray::ButtonName direction);

public:
        // This screen uses RIGHT
        JogModeScreen() : Screen(_BV((uint8_t)ButtonArray::RIGHT)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

class FilamentOdometerScreen: public Screen {

private:
	bool needsRedraw;

public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

class SDMenu: public Menu {

public:
	SDMenu();

	micros_t getUpdateRate() {return 50L * 1000L;}

	void resetState();

	void notifyButtonPressed(ButtonArray::ButtonName button);

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

private:
	bool    drawItemLockout;
	bool    selectable;
        uint8_t updatePhase;
        uint8_t updatePhaseDivisor;
	uint8_t lastItemIndex;
	int8_t  folderStackIndex;
	uint8_t folderStack[4];
};

class FinishedPrintMenu: public Menu {

public:
	FinishedPrintMenu();

	micros_t getUpdateRate() {return 50L * 1000L;}

	void resetState();

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};

#ifndef SINGLE_EXTRUDER

class SelectAlignmentMenu : public CounterMenu {

public:
	SelectAlignmentMenu();

protected:
	int8_t  counter[2];
        int32_t offsets[2];

	void resetState();

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void handleCounterUpdate(uint8_t index, int8_t up);
};

class NozzleCalibrationScreen: public Screen {

private:
	uint8_t alignmentState;
	bool needsRedraw;               ///< set to true if a menu item changes out of sequence

public:
	NozzleCalibrationScreen(): Screen(IS_STICKY_MASK) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

#endif

/// Display a welcome splash screen on first user boot

#pragma GCC diagnostic pop GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"

class PreheatSettingsMenu: public CounterMenu {

public:
        PreheatSettingsMenu();

protected:
	uint16_t counterRight;
	uint16_t counterLeft;
	uint16_t counterPlatform;
	uint8_t offset;

	void resetState();

	micros_t getUpdateRate() {return 50L * 1000L;}

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void handleCounterUpdate(uint8_t index, int8_t up);
};

class ResetSettingsMenu: public Menu {

public:
	ResetSettingsMenu();

	void resetState();

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
};


class ProfileChangeNameModeScreen: public Screen {

private:
        uint8_t cursorLocation;
        uint8_t profileName[PROFILE_NAME_SIZE+1];

public:
        ProfileChangeNameModeScreen() :
	     Screen(_BV((uint8_t)ButtonArray::UP) |
		    _BV((uint8_t)ButtonArray::DOWN) |
		    _BV((uint8_t)ButtonArray::RIGHT)) {}

        micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);

        uint8_t profileIndex;
};

class ProfileDisplaySettingsMenu: public Menu {

private:
        uint8_t profileName[8+1];
        uint32_t home[3];
        uint16_t hbpTemp, rightTemp, leftTemp;

public:
        ProfileDisplaySettingsMenu();

        void resetState();

        uint8_t profileIndex;

protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
};

class ProfileSubMenu: public Menu {
public:
        ProfileSubMenu();

        void resetState();

        uint8_t profileIndex;
protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);
};

class ProfilesMenu: public Menu {

public:
        ProfilesMenu();

        void resetState();

protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);
};

class HomeOffsetsModeScreen: public Screen {

private:
        enum homeOffState {
                HOS_NONE = 0,
                HOS_OFFSET_X,
                HOS_OFFSET_Y,
                HOS_OFFSET_Z,
        };

        enum homeOffState homeOffsetState, lastHomeOffsetState;

	bool valueChanged;

public:
	HomeOffsetsModeScreen(): Screen(_BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

        micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class MonitorModeScreen: public Screen {

private:
	uint8_t updatePhase;
	bool heating;
        bool resetLCD;

#if defined(BUILD_STATS) || defined(MODEL_REPLICATOR2)
	enum BuildTimePhase {
		BUILD_TIME_PHASE_FIRST = 0,
		BUILD_TIME_PHASE_ELAPSED_TIME = BUILD_TIME_PHASE_FIRST,
		BUILD_TIME_PHASE_TIME_LEFT,
		BUILD_TIME_PHASE_FILAMENT,
		BUILD_TIME_PHASE_ZPOS,
#ifdef ACCEL_STATS
		BUILD_TIME_PHASE_ACCEL_STATS,
#endif
		BUILD_TIME_PHASE_LAST	//Not counted, just an end marker
	};

	enum BuildTimePhase buildTimePhase, lastBuildTimePhase;
        uint32_t lastElapsedSeconds;
#endif

public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

extern MonitorModeScreen monitorModeScreen;

class HeaterPreheatMenu: public Menu {

public:
        HeaterPreheatMenu();

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

private:
	int8_t _rightActive, _leftActive, _platformActive;

	void storeHeatByte();
	void resetState();
	bool preheatActive;
};

class SettingsMenu: public CounterMenu {

public:
        SettingsMenu();
	micros_t getUpdateRate() {return 100L * 1000L;}

protected:
	void resetState();

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void handleCounterUpdate(uint8_t index, int8_t up);

private:
	/// Static instances of our menus

	bool singleExtruder;
	bool soundOn;
#ifdef HAS_RGB_LED
	bool heatingLEDOn;
#endif
	bool accelerationOn;
	bool overrideGcodeTempOn;
	bool pauseHeatOn;
	bool extruderHoldOn;
	bool toolOffsetSystemOld;
	bool useCRC;
#ifdef PSTOP_SUPPORT
	bool pstopEnabled;
#endif
#ifdef DITTO_PRINT
	bool dittoPrintOn;
#endif
#ifdef MACHINE_ID_MENU
        uint8_t bottype;
        uint16_t machine_id;
#endif
#ifdef ALTERNATE_UART
        bool altUART;
#endif
};

#ifdef EEPROM_MENU_ENABLE

class EepromMenu: public Menu {

public:
        EepromMenu();
        void resetState();

protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void notifyButtonPressed(ButtonArray::ButtonName button);

private:
	uint8_t safetyGuard;

	bool warningScreen;

	int8_t itemSelected;
};

#endif

class BotStatsScreen: public Screen {

public:
	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

class UtilitiesMenu: public Menu {

public:
	UtilitiesMenu();

	micros_t getUpdateRate() {return 100L * 1000L;}

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void resetState();

private:
	bool stepperEnable;
};


class MainMenu: public Menu {

public:
        MainMenu();

	micros_t getUpdateRate() {return 200L * 1000L;}

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

	void resetState();
};

#endif
