#ifndef INTERFACE_BOARD_HH_
#define INTERFACE_BOARD_HH_

#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"
#include "Menu.hh"

#define MENU_DEPTH 5

/**
 * Monitors the input state of a bunch of buttons, in polling mode.
 **/

namespace interfaceboard {

void init();

void pushScreen(Screen* newScreen);

void popScreen();

void doInterrupt();

void doUpdate();

micros_t getUpdateRate();

}

#endif
