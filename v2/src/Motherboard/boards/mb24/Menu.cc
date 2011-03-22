#include "Menu.hh"
#include "InterfaceBoard.hh"
#include "Types.hh"
#include "Steppers.hh"

void MonitorMode::reset() {
	pos = 0;
}

void MonitorMode::update(LiquidCrystal& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar welcome[] = "Welcome to";
	static PROGMEM prog_uchar minotaur[] =   "Minotaur mode!";

	if (forceRedraw) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.write_from_pgmspace(welcome);
		lcd.setCursor(0,1);
		lcd.write_from_pgmspace(minotaur);
	} else {
		if (pos < LCD_SCREEN_WIDTH -1) {
			lcd.setCursor(pos,3);
		}
		else {
			lcd.setCursor(LCD_SCREEN_WIDTH*2 -2 - pos,3);
		}
		lcd.write(' ');
	}

	pos += 1;
	if (pos >= LCD_SCREEN_WIDTH * 2 - 2) {
		pos = 0;
	}

	if (pos < LCD_SCREEN_WIDTH -1) {
		lcd.setCursor(pos,3);
	}
	else {
		lcd.setCursor(LCD_SCREEN_WIDTH*2 -2 - pos,3);
	}

	lcd.write('M');
}

void MonitorMode::notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button) {
	switch (button) {
	case InterfaceBoardDefinitions::CANCEL:
		interfaceboard::popScreen();
		break;
	}
}


void JogMode::reset() {
}

void JogMode::update(LiquidCrystal& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar jog[] = "Jog away!";

	if (forceRedraw) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.write_from_pgmspace(jog);
	}
}

void JogMode::jog(InterfaceBoardDefinitions::ButtonName direction) {
	Point position = steppers::getPosition();

	uint8_t steps = 50;
	int32_t interval = 2000;

	switch(direction) {
	case InterfaceBoardDefinitions::XMINUS:
		position[0] -= steps;
		break;
	case InterfaceBoardDefinitions::XPLUS:
		position[0] += steps;
		break;
	case InterfaceBoardDefinitions::YMINUS:
		position[1] -= steps;
		break;
	case InterfaceBoardDefinitions::YPLUS:
		position[1] += steps;
		break;
	case InterfaceBoardDefinitions::ZMINUS:
		position[2] -= steps;
		break;
	case InterfaceBoardDefinitions::ZPLUS:
		position[2] += steps;
		break;
	}

	steppers::setTarget(position, interval);
}

void JogMode::notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button) {
	switch (button) {
	case InterfaceBoardDefinitions::YMINUS:
	case InterfaceBoardDefinitions::ZMINUS:
	case InterfaceBoardDefinitions::YPLUS:
	case InterfaceBoardDefinitions::ZPLUS:
	case InterfaceBoardDefinitions::XMINUS:
	case InterfaceBoardDefinitions::XPLUS:
		jog(button);
		break;
	case InterfaceBoardDefinitions::CANCEL:
		interfaceboard::popScreen();
		break;
	}
}

void Menu::update(LiquidCrystal& lcd, bool forceRedraw) {
	// Do we need to redraw the whole menu?
	if ((itemIndex/LCD_SCREEN_HEIGHT) != (lastDrawIndex/LCD_SCREEN_HEIGHT)
			|| forceRedraw ) {
		// Redraw the whole menu
		lcd.clear();

		for (uint8_t i = 0; i < LCD_SCREEN_HEIGHT; i++) {
			if (i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT +1 > itemCount) {
				break;
			}

			lcd.setCursor(1,i);
			// Draw one page of items at a time
			drawItem(i+(itemIndex/LCD_SCREEN_HEIGHT)*LCD_SCREEN_HEIGHT, lcd);
		}
	}
	else {
		// Only need to clear the previous cursor
		lcd.setCursor(0,(lastDrawIndex%LCD_SCREEN_HEIGHT));
		lcd.write(' ');
	}

	lcd.setCursor(0,(itemIndex%LCD_SCREEN_HEIGHT));
	lcd.write('>');
	lastDrawIndex = itemIndex;
}

void Menu::reset() {
	itemIndex = 0;
	lastDrawIndex = 255;
}

void Menu::handleSelect(uint8_t index) {
}

void Menu::handleCancel() {
	// Remove ourselves from the menu list
	interfaceboard::popScreen();
}

void Menu::notifyButtonPressed(InterfaceBoardDefinitions::ButtonName button) {
	switch (button) {
	case InterfaceBoardDefinitions::ZERO:
	case InterfaceBoardDefinitions::OK:
		handleSelect(itemIndex);
		break;
	case InterfaceBoardDefinitions::CANCEL:
		handleCancel();
		break;
	case InterfaceBoardDefinitions::YMINUS:
	case InterfaceBoardDefinitions::ZMINUS:
		// increment index
		if (itemIndex < itemCount - 1) {
			itemIndex++;
		}
		break;
	case InterfaceBoardDefinitions::YPLUS:
	case InterfaceBoardDefinitions::ZPLUS:
		// decrement index
		if (itemIndex > 0) {
			itemIndex--;
		}
		break;

	case InterfaceBoardDefinitions::XMINUS:
	case InterfaceBoardDefinitions::XPLUS:
		break;
	}
}

MainMenu::MainMenu() {
	// TODO: Does this work?
//	(Menu)this->reset();
	itemIndex = 0;
	lastDrawIndex = 255;
	itemCount = 3;
}

void MainMenu::drawItem(uint8_t index, LiquidCrystal& lcd) {
	static PROGMEM prog_uchar monitor[] = "Monitor Mode";
	static PROGMEM prog_uchar build[] =   "Build from SD";
	static PROGMEM prog_uchar jog[] =   "Jog Mode";

	switch (index) {
	case 0:
		lcd.write_from_pgmspace(monitor);
		break;
	case 1:
		lcd.write_from_pgmspace(build);
		break;
	case 2:
		lcd.write_from_pgmspace(jog);
		break;
	}
}

void MainMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// Show monitor build screen
			interfaceboard::pushScreen(&monitor);
			break;
		case 1:
			// Show build from SD screen
			interfaceboard::pushScreen(&sdMenu);
			break;
		case 2:
			// Show build from SD screen
			interfaceboard::pushScreen(&jogger);
			break;
		}
}

SDMenu::SDMenu() {
	itemCount = 0;
	itemIndex = 0;
	lastDrawIndex = 255;
}

void SDMenu::reset() {
	itemIndex = 0;
	itemCount = countFiles();
}

// Count the number of files on the SD card
uint8_t SDMenu::countFiles() {
	uint8_t count = 0;

	sdcard::SdErrorCode e;

	// First, reset the directory index
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
		// TODO: Report error
		return 6;
	}

	const int MAX_FILE_LEN = 2;
	char fnbuf[MAX_FILE_LEN];

	// Count the files
	do {
		e = sdcard::directoryNextEntry(fnbuf,MAX_FILE_LEN);
		if (fnbuf[0] == '\0') {
			break;
		}

		// If it's a dot file, don't count it.
		if (fnbuf[0] == '.') {
		}
		else {
			count++;
		}
	} while (e == sdcard::SD_SUCCESS);

	// TODO: Check for error again?

	return count;
}

sdcard::SdErrorCode SDMenu::getFilename(uint8_t index, char buffer[], uint8_t buffer_size) {
	sdcard::SdErrorCode e;

	// First, reset the directory list
	e = sdcard::directoryReset();
	if (e != sdcard::SD_SUCCESS) {
		return e;
	}


	for(uint8_t i = 0; i < index+1; i++) {
		// Ignore dot-files
		do {
			e = sdcard::directoryNextEntry(buffer,buffer_size);
			if (buffer[0] == '\0') {
				return e;
			}
		} while (e == sdcard::SD_SUCCESS && buffer[0] == '.');

		if (e != sdcard::SD_SUCCESS) {
			return e;
		}
	}

	return e;
}

void SDMenu::drawItem(uint8_t index, LiquidCrystal& lcd) {
	if (index > itemCount - 1) {
		// TODO: report error
		return;
	}

	const uint8_t MAX_FILE_LEN = LCD_SCREEN_WIDTH;
	char fnbuf[MAX_FILE_LEN];

	sdcard::SdErrorCode e;
	e = getFilename(index, fnbuf, MAX_FILE_LEN);

	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return;
	}

	uint8_t idx;
	for (idx = 0; (idx < MAX_FILE_LEN) && (fnbuf[idx] != 0); idx++) {
		lcd.write(fnbuf[idx]);
	}
}

void SDMenu::handleSelect(uint8_t index) {
	// TODO: what's a good max file length?
	const int MAX_FILE_LEN = 32;
	char fnbuf[MAX_FILE_LEN];
	sdcard::SdErrorCode e;
	e = getFilename(index, fnbuf, MAX_FILE_LEN);
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return;
	}

	e = sdcard::startPlayback(fnbuf);
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return;
	}

	// TODO: Jump to build monitor here
}
