#ifndef INTERFACE_BOARD_HH_
#define INTERFACE_BOARD_HH_

#include "InterfaceBoardDefinitions.hh"
#include "LiquidCrystal.hh"
#include "Menu.hh"
#include "ButtonArray.hh"

#define MENU_DEPTH 5

/**
 * Monitors the input state of a bunch of buttons, in polling mode.
 **/

namespace interfaceboard {

void init();

void pushMenu(Menu* newMenu);

void popMenu();

void doInterrupt();

}

#endif
