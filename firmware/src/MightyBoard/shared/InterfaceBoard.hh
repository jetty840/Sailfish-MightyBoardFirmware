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


#ifndef INTERFACE_BOARD_HH_
#define INTERFACE_BOARD_HH_

#include "Configuration.hh"
#include "Pin.hh"
#include "ButtonArray.hh"
#include "Menu.hh"

/// Maximum number of screens that can be active at once.
#define SCREEN_STACK_DEPTH      7



/// The InterfaceBoard module provides support for the MakerBot Industries
/// Gen4 Interface Board. It could very likely be adopted to support other
/// LCD/button setups as well.
/// \ingroup HardwareLibraries
class InterfaceBoard {
public:
    LiquidCrystalSerial& lcd;              ///< LCD to write to
        
        
		enum ScreenType{
			BUILD_FINISHED = 1,
			MESSAGE_SCREEN = 2,
		};

private:
        ButtonArray& buttons;            ///< Button array to read from

        MonitorMode buildScreen;            ///< Screen to display while building
        MainMenu mainScreen;            ///< Root menu screen
        MessageScreen messageScreen;		 ///< Screen to display messages
        
        SnakeMode snake;				///< Snake game
        
        BuildFinished buildFinished;	///< screen displayed at end of build

        /// Stack of screens to display; the topmost one will actually
        /// be drawn to the screen, while the other will remain resident
        /// but not active.
        Screen* screenStack[SCREEN_STACK_DEPTH];
        int8_t screenIndex;             ///< Stack index of the current screen.

        Pin LEDs[2];                    ///< Pins connected to the LEDs

        bool building;                  ///< True if the bot is building
        
        uint8_t buildPercentage;

        uint8_t waitingMask;            ///< Mask of buttons the interface is
                                        ///< waiting on.
    
        bool screen_locked;             /// set to true in case of catastrophic failure (ie heater cuttoff triggered)
        
        uint8_t onboard_start_idx;		/// screen stack index when onboard script is started
public:
        /// Construct an interface board.
        /// \param[in] button array to read from
        /// \param[in] LCD to display on
        /// \param[in] Pin connected to the foo LED
        /// \param[in] Pin connected to the bar LED
        /// \param[in] Main screen, shown as root display
        /// \param[in] Screen to display while building
        InterfaceBoard(ButtonArray& buttons_in,
                       LiquidCrystalSerial& lcd_in,
                       const Pin& gled,
                       const Pin& rled);

        /// Initialze the interface board. This needs to be called once
        /// at system startup (or reset).
        void init();

        /// This should be called periodically by a high-speed interrupt to
        /// service the button input pad.
        void doInterrupt();

        /// Add a new screen to the stack. This automatically calls reset()
        /// and then update() on the screen, to ensure that it displays
        /// properly. If there are more than SCREEN_STACK_DEPTH screens already
        /// in the stack, than this function does nothing.
        /// \param[in] newScreen Screen to display.
        void pushScreen(Screen* newScreen);
        
        /// set the build percentage to be displayed in monitor mode
        void setBuildPercentage(uint8_t percent);

        /// Remove the current screen from the stack. If there is only one screen
        /// being displayed, then this function does nothing.
        void popScreen();
          
        /// Remove the top two screen from the stack. this ensures smooth transition between screens 
        /// when 2 are popped at once
        void pop2Screens();

        /// Return a pointer to the currently displayed screen.
        Screen* getCurrentScreen() { return screenStack[screenIndex]; }

        micros_t getUpdateRate();

        void doUpdate();

        void showMonitorMode();
        
        void setLED(uint8_t id, bool on);

        /// Tell the interface board that the system is waiting for a button push
        /// corresponding to one of the bits in the button mask. The interface board
        /// will not process button pushes directly until one of the buttons in the
        /// mask is pushed.
        void waitForButton(uint8_t button_mask);
        
        /// Check if the expected button push has been made. If waitForButton was
        /// never called, always return true.
        bool buttonPushed();
        
        /// push Error Message Screen
        void errorMessage(char buf[]);
          
        /// lock screen so that no pushes/pops can occur
        /// used in the case of heater failure to force restart
        void lock(){ screen_locked = true;}
        
        /// re-initialize LCD
        void resetLCD();
        
        /// push a local screen
        void queueScreen(ScreenType screen);
        
        /// record screen stack index when onboard script is started so we can return there on finish
        void RecordOnboardStartIdx();
        
        /// pop screen without refreshing the new head screen
        void popScreenQuick();
        
        /// pop Error Message Screen
        void DoneWithMessage();

        // return a pointer to the message screen
        MessageScreen* GetMessageScreen();

};

#endif
