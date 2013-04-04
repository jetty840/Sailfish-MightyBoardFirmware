/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef BOARDS_MB40_MOTHERBOARD_HH_
#define BOARDS_MB40_MOTHERBOARD_HH_

#include "UART.hh"
#include "DigiPots.hh"
#include "Types.hh"
#include "PSU.hh"
#include "Configuration.hh"
#include "Timeout.hh"
#include "Menu.hh"
#include "InterfaceBoard.hh"
#include "LiquidCrystalSerial.hh"
#include "ButtonArray.hh"
#include "Thermistor.hh"
#include "HeatingElement.hh"
#include "Heater.hh"
#include "ExtruderBoard.hh"
#ifdef MODEL_REPLICATOR
#include "Cutoff.hh"
#endif

#ifdef DEBUG_VALUE
	void setDebugValue(uint8_t value);
#endif

#include "StepperAxis.hh"
#include "StepperAccelPlanner.hh"

/// Build platform heating element on v34 Extruder controller
/// \ingroup ECv34
class BuildPlatformHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};


extern uint8_t board_status;
#define BOARD_STATUS_SET(x) ( board_status |= (x) )
#define BOARD_STATUS_CLEAR(x) ( board_status &= ~(x) )

/// Main class for Motherboard version 4.0+ (Gen4 electronics)
/// \ingroup HardwareLibraries
/// \ingroup MBv40
class Motherboard {
private:
        // TODO: Declare this in main, drop the singleton.
        /// Static instance of the motherboard
        static Motherboard motherboard;

public:

	enum status_states{
		STATUS_NONE = 0,
		STATUS_HEAT_INACTIVE_SHUTDOWN = 0x40,
		STATUS_CANCELLING = 0x20,
		STATUS_WAITING_FOR_BUTTON = 0x10,
		STATUS_ONBOARD_PROCESS = 0x08,
		STATUS_ONBOARD_SCRIPT = 0x04,
		STATUS_MANUAL_MODE = 0x02,
		STATUS_PREHEATING = 0x01
	};

        /// Get the motherboard instance.
        static Motherboard& getBoard() { return motherboard; }

	static void heatersOff(bool platform);
	static void pauseHeaters(bool pause);
	static void interfaceBlinkOn();
	static void interfaceBlinkOff();

        ExtruderBoard& getExtruderBoard(uint8_t id) { if(id == 1){ return Extruder_Two;} else  { return Extruder_One;} }
#ifdef MODEL_REPLICATOR2
	ThermocoupleReader& getThermocoupleReader() { return therm_sensor; }
#endif	
	void initClocks();

private:
	/// Private constructor; use the singleton
	Motherboard();
	
        // TODO: Move this to an interface board slice.
	Timeout interface_update_timeout;
#ifdef MODEL_REPLICATOR2
	Timeout therm_sensor_timeout;
	ThermocoupleReader therm_sensor;
#else
	Cutoff cutoff; //we're not using the safety cutoff, but we need to disable the circuit
#endif
	Timeout extruder_manage_timeout;
	Timeout platform_timeout;

        /// True if we have an interface board attached
	bool hasInterfaceBoard;

	LiquidCrystalSerial lcd;

	MessageScreen messageScreen;    ///< Displayed by user-specified messages

public:
	Timeout user_input_timeout;
	MainMenu mainMenu;              ///< Main system menu
	FinishedPrintMenu finishedPrintMenu;
	InterfaceBoard interfaceBoard;
	Thermistor platform_thermistor;
	Heater platform_heater;
	bool using_platform;
	
	ExtruderBoard Extruder_One;
	ExtruderBoard Extruder_Two;

#ifdef PSTOP_SUPPORT
	uint8_t pstop_enabled;
#endif

	ButtonArray buttonArray;
	
	BuildPlatformHeatingElement platform_element;

	bool buttonWait;
	bool reset_request;
	uint8_t heatShutdown;  // set if safety cutoff is triggered
	HeaterFailMode heatFailMode;

        //2 types of stepper timers depending on if we're using accelerated or not
        void setupAccelStepperTimer();

	/// Reset the motherboard to its initial state.
	/// This only resets the board, and does not send a reset
	/// to any attached toolheads.
	void reset(bool hard_reset);

	/// initialize things that only need to be set up once, on boot
	void init();

	void runMotherboardSlice();

	/// Count the number of steppers available on this board.
        const int getStepperCount() const { return STEPPER_COUNT; }

	/// Get the number of microseconds that have passed since
	/// the board was initialized.  This value will wrap after
	/// 2**32 microseconds (ca. 70 minutes); callers should compensate for this.
	micros_t getCurrentMicros();

#if defined(HONOR_DEBUG_PACKETS) && (HONOR_DEBUG_PACKETS == 1)
	/// Write an error code to the debug pin.
	void indicateError(int errorCode);

	/// Get the current error being displayed.
	uint8_t getCurrentError();	
#endif

	/// set the interface LEDs to blink
	void interfaceBlink(uint8_t on_time, uint8_t off_time);

	/// Perform the stepper interrupt routine.
	void doStepperInterrupt();

	bool isUsingPlatform() { return using_platform; }
	void setUsingPlatform(bool is_using);
	void setExtra(bool on);
	Heater& getPlatformHeater() { return platform_heater; }

	InterfaceBoard& getInterfaceBoard() { return interfaceBoard; }	

	MessageScreen* getMessageScreen() { return &messageScreen; }

	void resetUserInputTimeout();
	void startButtonWait();
	void heaterFail(HeaterFailMode mode, uint8_t slave_id);
	/// push an error screen, and wait until button
	void errorResponse(const prog_uchar *msg, bool reset = false, bool incomplete = false);
	void errorResponse(const prog_uchar *msg1, const prog_uchar *msg2, bool reset = false, bool incomplete = false);

	/// set board_status flag
	/// void setBoardStatus(status_states state, bool on);

	void HeatingAlerts();
};

#endif // BOARDS_MB40_MOTHERBOARD_HH_
