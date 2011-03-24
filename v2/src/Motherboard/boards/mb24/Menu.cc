#include "Menu.hh"
#include "InterfaceBoard.hh"
#include "Types.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "Errors.hh"
#include "Tool.hh"

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

void MonitorMode::reset() {
	pos = 0;
}

// taken from Host.cc
inline uint8_t handleToolQuery(const uint8_t packetData[], uint8_t length, OutPacket& to_host) {
	// Quick sanity assert: ensure that host packet length >= 2
	// (Payload must contain toolhead address and at least one byte)
	if (length < 2) {
		return ERR_HOST_TRUNCATED_CMD;
	}
	Timeout acquire_lock_timeout;
	acquire_lock_timeout.start(HOST_TOOL_RESPONSE_TIMEOUT_MS);
	while (!tool::getLock()) {
		if (acquire_lock_timeout.hasElapsed()) {
			return ERR_SLAVE_LOCK_TIMEOUT;
		}
	}
	OutPacket& out = tool::getOutPacket();
	InPacket& in = tool::getInPacket();
	out.reset();
//	for (int i = 1; i < from_host.getLength(); i++) {
//		out.append8(from_host.read8(i));
//	}
	for (uint8_t i = 0; i < length; i++) {
		out.append8(packetData[i]);
	}
	// Timeouts are handled inside the toolslice code; there's no need
	// to check for timeouts on this loop.
	tool::startTransaction();
	tool::releaseLock();
	// WHILE: bounded by tool timeout in runToolSlice
	while (!tool::isTransactionDone()) {
		tool::runToolSlice();
	}
	if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
		return ERR_SLAVE_PACKET_TIMEOUT;
	} else {
		// Copy payload back. Start from 0-- we need the response code.
		for (uint8_t i = 0; i < in.getLength(); i++) {
			to_host.append8(in.read8(i));
		}
	}
	return NO_ERROR;
}

void MonitorMode::update(LiquidCrystal& lcd, bool forceRedraw) {
	static PROGMEM prog_uchar monitor[] =   "Build Monitor";
	static PROGMEM prog_uchar extruder_temp[] =   "Tool: ";
	static PROGMEM prog_uchar platform_temp[] =   "Bed:  ";

	if (forceRedraw) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.write_from_pgmspace(monitor);
	} else {
	}

	// Query the tool for the latest temperature
	{
		OutPacket outPacket;

		uint8_t packetData[] = {0, SLAVE_CMD_GET_TEMP};
		uint8_t length = 2;

		uint8_t error = handleToolQuery(packetData, length, outPacket);

		if (error == NO_ERROR) {
			lcd.setCursor(0,2);
			lcd.write_from_pgmspace(extruder_temp);
			int16_t temp = outPacket.read16(1);
			lcd.write((temp%1000)/100+'0');
			lcd.write((temp%100)/10+'0');
			lcd.write((temp%10)+'0');
			lcd.write('C');
		}
	}
	// Query the bed for the latest temperature
	{
		OutPacket outPacket;

		uint8_t packetData[] = {0, SLAVE_CMD_GET_PLATFORM_TEMP};
		uint8_t length = 2;

		uint8_t error = handleToolQuery(packetData, length, outPacket);

		if (error == NO_ERROR) {
			lcd.setCursor(0,3);
			lcd.write_from_pgmspace(platform_temp);
			int16_t temp = outPacket.read16(1);
			lcd.write((temp%1000)/100+'0');
			lcd.write((temp%100)/10+'0');
			lcd.write((temp%10)+'0');
			lcd.write('C');
		}
	}
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
