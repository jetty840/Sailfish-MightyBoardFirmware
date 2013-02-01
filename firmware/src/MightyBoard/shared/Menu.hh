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

enum FilamentStates{
    FILAMENT_HEATING,
    FILAMENT_HEAT_BAR,
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

enum AlignmentState {
	ALIGNMENT_START,
	ALIGNMENT_PRINT,
	ALIGNMENT_EXPLAIN1,
	ALIGNMENT_EXPLAIN2,
	ALIGNMENT_SELECT,
	ALIGNMENT_END,
	ALIGNMENT_QUIT
};


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

#define CONTINUOUS_BUTTONS_MASK	0b00011111
#define IS_STICKY_MASK 		_BV(6)
#define IS_CANCEL_SCREEN_MASK	_BV(7)

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

	Screen(uint8_t oMask): optionsMask(oMask) {}
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
		bool lineUpdate;				///< flags the menu to update the current line
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

        /// Handle the menu being cancelled. This should either remove the
        /// menu from the stack, or pop up a confirmation dialog to make sure
        /// that the menu should be removed.
	void handleCancel();
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

#ifdef PARFAIT
    void drawItem(uint8_t index, LiquidCrystalSerial& lcd) { }
    void handleCounterUpdate(uint8_t index, bool up) { }
#else
    void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    virtual void handleCounterUpdate(uint8_t index, bool up);
#endif
};

/// Display a welcome splash screen, that removes itself when updated.
class SplashScreen: public Screen {

private:
	bool hold_on;
public:
	SplashScreen(uint8_t optionsMask): Screen(optionsMask), hold_on(false) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

	void SetHold(bool on);
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
};

class CancelBuildMenu: public Menu {
public:
	CancelBuildMenu(uint8_t optionsMask);
    
	void resetState();
    
    bool isCancelScreen(){return true;}
	
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
    
    
    bool paused;
};

class BuildStats: public Screen {

private:

	const static uint8_t UPDATE_COUNT_MAX = 8;
	uint8_t update_count;

public:
	BuildStats(uint8_t optionsMask): Screen(optionsMask) {}

	micros_t getUpdateRate() {return 500L * 1000L;}
	
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
	
	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
};

/// load / unload filament options
class FilamentScreen: public Screen {
    
public:
	FilamentScreen(uint8_t optionsMask): Screen(optionsMask) {}

	micros_t getUpdateRate() {return 500L * 1000L;}
    
    void setScript(FilamentScript script);
    
    
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
    
	void reset();
    
    void notifyButtonPressed(ButtonArray::ButtonName button);
    
private:
    Timeout filamentTimer;
    uint16_t lastHeatIndex;
    int toggleCounter;
    int filamentTemp[EXTRUDERS];
    uint8_t filamentState;
    uint8_t axisID, toolID;
    uint8_t digiPotOnEntry;
    uint8_t restoreAxesEnabled;
    bool forward;
    bool heatLights;
    bool LEDClear;
    bool helpText;
    bool toggleBlink;
    bool needsRedraw;
    
    void startMotor();
    void stopMotor();
};

class FilamentMenu: public Menu {
public:
	FilamentMenu(uint8_t optionsMask);
    
    	/// Static instances of our menus
    	FilamentScreen filament;
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
	
	void resetState();    
};

class PauseAtZPosScreen: public Screen {
private:
	float pauseAtZPos;

public:
	PauseAtZPosScreen(uint8_t optionsMask): Screen(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class ActiveBuildMenu: public Menu {
	
private:
	BuildStats build_stats_screen;
	PauseAtZPosScreen pauseAtZPosScreen;
	
	//Fan ON/OFF
	//LEDS OFF / COLORS

	bool is_paused;

public:
	ActiveBuildMenu(uint8_t optionsMask);
    
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
	const static int BUF_SIZE = LCD_SCREEN_WIDTH*LCD_SCREEN_HEIGHT + 1;
	char message[BUF_SIZE];
	uint8_t cursor;
	bool needsRedraw;
	bool incomplete;
	Timeout timeout;
    
public:
	MessageScreen(uint8_t optionsMask) : Screen(optionsMask), needsRedraw(false) { message[0] = '\0'; }

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


class JogMode: public Screen {
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
	bool distanceChanged, modeChanged;
	jogmode_t  JogModeScreen;
	bool jogging;

    void jog(ButtonArray::ButtonName direction);

public:
	JogMode(uint8_t optionsMask): Screen(optionsMask) {}

	micros_t getUpdateRate() {return 50L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
};

class FilamentOdometerScreen: public Screen {
private:
	bool needsRedraw;

public:
	FilamentOdometerScreen(uint8_t optionsMask): Screen(optionsMask) {}

	micros_t getUpdateRate() {return 500L * 1000L;}

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

	void notifyButtonPressed(ButtonArray::ButtonName button);
};

/*class SDSpecialBuild: public Screen{
	
	protected:
		char buildType[host::MAX_FILE_LEN];
		bool buildFailed;
		

public:
	SDSpecialBuild();
	micros_t getUpdateRate() {return 50L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();
	virtual void resetState();
	
	bool startBuild(void);
	void notifyButtonPressed(ButtonArray::ButtonName button);

};*/

class SDMenu: public Menu {
public:
	SDMenu(uint8_t optionsMask);

	micros_t getUpdateRate() {return 50L * 1000L;}

	void resetState();
	
	void notifyButtonPressed(ButtonArray::ButtonName button);

	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

protected:
	uint8_t countFiles();

	bool getFilename(uint8_t index,
                         char buffer[],
                         uint8_t buffer_size,
			 uint8_t *buflen,
			 bool *isdir);

	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

private:
        uint8_t updatePhase;
        uint8_t updatePhaseDivisor;
	uint8_t lastItemIndex;
	bool    drawItemLockout;
	bool    selectable;
	int8_t  folderStackIndex;
	uint8_t folderStack[4];
};

class SelectAlignmentMenu : public CounterMenu{
	
public:
	SelectAlignmentMenu(uint8_t optionsMask);
    
protected:
    int8_t xCounter;
    int8_t yCounter;
    
    void resetState();
    
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
    
    void handleCounterUpdate(uint8_t index, bool up);
};

class NozzleCalibrationScreen: public Screen {
	
private:
    SelectAlignmentMenu align;
    
    uint8_t alignmentState;
    bool needsRedraw;               ///< set to true if a menu item changes out of sequence
	
public:
	NozzleCalibrationScreen(uint8_t optionsMask): Screen(optionsMask | IS_STICKY_MASK), align((uint8_t)0) {}

	micros_t getUpdateRate() {return 50L * 1000L;}
    
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
    
	void reset();
    
    void notifyButtonPressed(ButtonArray::ButtonName button);
};

/// Display a welcome splash screen on first user boot

#pragma GCC diagnostic pop GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"

class PreheatSettingsMenu: public CounterMenu {
public:
	PreheatSettingsMenu(uint8_t optionsMask);
    
protected:
    uint16_t counterRight;
    uint16_t counterLeft;
    uint16_t counterPlatform;
    
    void resetState();

	micros_t getUpdateRate() {return 50L * 1000L;}
    
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
    
    void handleCounterUpdate(uint8_t index, bool up);
};

class ResetSettingsMenu: public Menu {
public:
	ResetSettingsMenu(uint8_t optionsMask);
    
	void resetState();
    
protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
};


class ProfileChangeNameMode: public Screen {
private:
        uint8_t cursorLocation;
        uint8_t profileName[PROFILE_NAME_SIZE+1];

public:
	ProfileChangeNameMode(uint8_t optionsMask): Screen(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

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
	ProfileDisplaySettingsMenu(uint8_t optionsMask);

        void resetState();

        uint8_t profileIndex;
protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);
};

class ProfileSubMenu: public Menu {
private:
        ProfileChangeNameMode      profileChangeNameMode;
        ProfileDisplaySettingsMenu profileDisplaySettingsMenu;

public:
	ProfileSubMenu(uint8_t optionsMask);

        void resetState();

        uint8_t profileIndex;
protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);
};

class ProfilesMenu: public Menu {
private:
        ProfileSubMenu profileSubMenu;
public:
	ProfilesMenu(uint8_t optionsMask);

        void resetState();
protected:
        void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

        void handleSelect(uint8_t index);
};

class HomeOffsetsMode: public Screen {
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
	HomeOffsetsMode(uint8_t optionsMask): Screen(optionsMask | _BV((uint8_t)ButtonArray::UP) | _BV((uint8_t)ButtonArray::DOWN)) {}

        micros_t getUpdateRate() {return 50L * 1000L;}

        void update(LiquidCrystalSerial& lcd, bool forceRedraw);

        void reset();

        void notifyButtonPressed(ButtonArray::ButtonName button);
};

class MonitorMode: public Screen {
private:
	ActiveBuildMenu active_build_menu;

	uint8_t updatePhase;
    bool toggleBlink;
    bool heating;
    bool LEDClear;
    bool heatLights;
    uint16_t lastHeatIndex;

#ifdef MODEL_REPLICATOR2
    enum BuildTimePhase {
	    BUILD_TIME_PHASE_FIRST = 0,
	    BUILD_TIME_PHASE_ELAPSED_TIME = BUILD_TIME_PHASE_FIRST,
	    BUILD_TIME_PHASE_TIME_LEFT,
	    BUILD_TIME_PHASE_ZPOS,
	    BUILD_TIME_PHASE_FILAMENT,
#ifdef ACCEL_STATS
	    BUILD_TIME_PHASE_ACCEL_STATS,
#endif
	    BUILD_TIME_PHASE_LAST	//Not counted, just an end marker
    };

    enum BuildTimePhase buildTimePhase, lastBuildTimePhase;
    float lastElapsedSeconds;
#endif

public:
	MonitorMode(uint8_t optionsMask): Screen(optionsMask), active_build_menu((uint8_t)0), cancelBuildMenu((uint8_t)0) {}

	CancelBuildMenu cancelBuildMenu;

	micros_t getUpdateRate() {return 500L * 1000L;}


	void update(LiquidCrystalSerial& lcd, bool forceRedraw);

	void reset();

    void notifyButtonPressed(ButtonArray::ButtonName button);
};

class HeaterPreheat: public Menu {
	
public:
	
	HeaterPreheat(uint8_t optionsMask);
	
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);

private:
	MonitorMode monitorMode;
	int8_t _rightActive, _leftActive, _platformActive;
    
    void storeHeatByte();
    void resetState();
    bool preheatActive;
};

class SettingsMenu: public CounterMenu {
public:
	SettingsMenu(uint8_t optionsMask);

	micros_t getUpdateRate() {return 100L * 1000L;}

protected:
	void resetState();
    
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);
    
	void handleSelect(uint8_t index);
	
	void handleCounterUpdate(uint8_t index, bool up);
    
private:
    /// Static instances of our menus
    
    bool singleExtruder;
    bool soundOn;
    bool heatingLEDOn;
    bool helpOn;
    bool accelerationOn;
    bool overrideGcodeTempOn; 
    bool pauseHeatOn; 
    bool extruderHoldOn;
    bool toolOffsetSystemOld;
#ifdef DITTO_PRINT
    bool dittoPrintOn;
#endif
    int8_t LEDColor;
};

#ifdef EEPROM_MENU_ENABLE

class EepromMenu: public Menu {
public:
	EepromMenu(uint8_t optionsMask);

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
	BotStatsScreen(uint8_t optionsMask): Screen(optionsMask) {}
	micros_t getUpdateRate() {return 500L * 1000L;}
	void update(LiquidCrystalSerial& lcd, bool forceRedraw);
	void reset();
	void notifyButtonPressed(ButtonArray::ButtonName button);
};

class UtilitiesMenu: public Menu {
public:
	UtilitiesMenu(uint8_t optionsMask);

	micros_t getUpdateRate() {return 100L * 1000L;}

	MonitorMode monitorMode;
	SplashScreen splash;
    	FilamentMenu filament;
	BotStatsScreen botStats;

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
	
	void resetState();

private:
    /// Static instances of our menus
    JogMode jogger;
 //   HeaterTestScreen heater;
    SettingsMenu set;
    PreheatSettingsMenu preheat;
    ProfilesMenu profilesMenu;
    HomeOffsetsMode homeOffsetsMode;
    ResetSettingsMenu reset_settings;
    NozzleCalibrationScreen alignment;
    FilamentOdometerScreen filamentOdometer;
#ifdef EEPROM_MENU_ENABLE
    EepromMenu eepromMenu;
#endif
    
    bool stepperEnable;
    bool blinkLED;
};


class MainMenu: public Menu {
public:
	MainMenu(uint8_t optionsMask);

	micros_t getUpdateRate() {return 200L * 1000L;}

        UtilitiesMenu utils;

        SDMenu sdMenu;

protected:
	void drawItem(uint8_t index, LiquidCrystalSerial& lcd);

	void handleSelect(uint8_t index);
	
	void resetState();

private:
        /// Static instances of our menus
        HeaterPreheat preheat;
};



#endif
