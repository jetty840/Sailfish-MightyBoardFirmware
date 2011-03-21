#include "Menu.hh"

void Menu::draw(LiquidCrystal& lcd) {
	// Do we need to redraw the whole menu?
	if ((itemIndex/LCD_SCREEN_HEIGHT) != (lastDrawIndex/LCD_SCREEN_HEIGHT)) {
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
	static PROGMEM prog_uchar info[] =    "Machine info";

	switch (index) {
	case 0:
		lcd.write_from_pgmspace(monitor);
		break;
	case 1:
		lcd.write_from_pgmspace(build);
		break;
	case 2:
		lcd.write_from_pgmspace(info);
		break;
	}
}

void MainMenu::handleSelect(uint8_t index) {
	switch (index) {
		case 0:
			// Show monitor build screen
			break;
		case 1:
			// Show build from SD screen
			break;
		case 2:
			// Show machine info screen
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

	sdcard::startPlayback(fnbuf);
	// TODO: Deal with error
}
