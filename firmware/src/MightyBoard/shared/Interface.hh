/*
 * Copyright 2011 by Matt Mets <matt.mets@makerbot.com>
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



#ifndef INTERFACE_HH_
#define INTERFACE_HH_

#include "Menu.hh"
#include "InterfaceBoard.hh"
#include "LiquidCrystalSerial.hh"
#include "Types.hh"

// TODO: This style interface is weird; find a way to replace it.
namespace interface {

/// Set the current interface board and lcd. This *must* be called before using
/// any of the functions in this interface.
void init(InterfaceBoard* board_in, LiquidCrystalSerial* lcd_in);

/// Returns true if the interface board is connected
bool isConnected();

/// Display a new screen by pushing it to the screen stack.
/// \param[in] newScreen Screen to be added to the stack.
void pushScreen(Screen* newScreen);

/// Remove the top screen from the screen stack. If there is only one screen left,
/// it will not be removed.
void popScreen();


/// Screen update interrupt. This scans the keypad to look for any changes. To
/// ensure a consistant user response, it should be called from a medium frequency
/// interrupt.
void doInterrupt();

/// Update the display. This function is where the current display screen
/// should handle button presses, redraw it's screen, etc. It is run from the
/// motherboard slice, not an interrupt, because it may take a relatively long
/// time to run.
void doUpdate();

/// Returns the minimum amount of time that should elapse before the current
/// display screen is updated again. This is customizable to allow for both
/// fast-updating interactive screens (jog mode, etc) that can be used when
/// the machine is not printing, as well as slow-updating screens (monitor
/// mode) that can be displayed while the machine is running, without causing
/// much impact.
micros_t getUpdateRate();

/// Set Interface board LEDS
void setLEDs(bool on);

/// set build percentage to be displayed in monitor mode
void setBuildPercentage(uint8_t percent);



}

#endif
