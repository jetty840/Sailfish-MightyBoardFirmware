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
#include "Cutoff.hh"

#define THERMOCOUPLE_UPDATE_TIMEOUT	250000 ///250ms - read 4 times per second (2 per channel)


/// Build platform heating element on v34 Extruder controller
/// \ingroup ECv34
class BuildPlatformHeatingElement : public HeatingElement {
public:
	void setHeatingElement(uint8_t value);
};

/// Main class for Motherboard version 4.0+ (Gen4 electronics)
/// \ingroup HardwareLibraries
/// \ingroup MBv40
class Motherboard {
private:
        // TODO: Declare this in main, drop the singleton.
        /// Static instance of the motherboard
        static Motherboard motherboard;

public:
        /// Get the motherboard instance.
        static Motherboard& getBoard() { return motherboard; }
        ExtruderBoard& getExtruderBoard(uint8_t id) { if(id == 1){ return Extruder_Two;} else  { return Extruder_One;} }

private:

	/// Microseconds since board initialization
	volatile micros_t micros;

	/// Private constructor; use the singleton
	Motherboard();

        // TODO: Move this to an interface board slice.
	Timeout interface_update_timeout;
	Timeout user_input_timeout;
	Timeout therm_sensor_timeout;

        /// True if we have an interface board attached
	bool hasInterfaceBoard;
	
	ExtruderBoard Extruder_One;
	ExtruderBoard Extruder_Two;
	
	ButtonArray buttonArray;
	LiquidCrystalSerial lcd;
	InterfaceBoard interfaceBoard;
	
	MainMenu mainMenu;              ///< Main system menu
	SplashScreen splashScreen;      ///< Displayed at startup
	MonitorMode monitorMode;        ///< Displayed during build
	WelcomeScreen welcomeScreen;	///< Displayed on Startup for the first time
	MessageScreen messageScreen;    ///< Displayed by user-specified messages
    
	Thermistor platform_thermistor;
	BuildPlatformHeatingElement platform_element;
	Heater platform_heater;
	bool using_platform;
	Thermocouple therm_sensor;

	
	Cutoff cutoff;
	bool heatShutdown;  // set if safety cutoff is triggered
	bool buttonWait;
	bool reset_request;
	HeaterFailMode heatFailMode;

public:
	/// Reset the motherboard to its initial state.
	/// This only resets the board, and does not send a reset
	/// to any attached toolheads.
	void reset(bool hard_reset);

	void runMotherboardSlice();

	/// Count the number of steppers available on this board.
        const int getStepperCount() const { return STEPPER_COUNT; }
	
	/// Get the number of microseconds that have passed since
	/// the board was initialized.  This value will wrap after
	/// 2**32 microseconds (ca. 70 minutes); callers should compensate for this.
	micros_t getCurrentMicros();

	/// Write an error code to the debug pin.
	void indicateError(int errorCode);
	/// Get the current error being displayed.
	uint8_t getCurrentError();
	
	/// set the interface LEDs to blink
	void interfaceBlink(int on_time, int off_time);

	/// Perform the timer interrupt routine.
	void doInterrupt();
	
	bool isUsingPlatform() { return using_platform; }
	void setUsingPlatform(bool is_using);
	void setValve(bool on);
	Heater& getPlatformHeater() { return platform_heater; }

	InterfaceBoard& getInterfaceBoard() { return interfaceBoard; }	

	MessageScreen* getMessageScreen() { return &messageScreen; }
	
	void resetUserInputTimeout();
	void startButtonWait();
	void heaterFail(HeaterFailMode mode);
	/// push an error screen, and wait until button 
	void errorResponse(char msg[], bool reset = false);
};



#endif // BOARDS_MB40_MOTHERBOARD_HH_
