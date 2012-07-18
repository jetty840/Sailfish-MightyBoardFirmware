#include "LiquidCrystalSerial.hh"
#include "Configuration.hh"

#include <stdio.h>
#include <string.h>
#include <util/delay.h>

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

LiquidCrystalSerial::LiquidCrystalSerial(Pin strobe, Pin data, Pin CLK) 
{
  init(strobe, data, CLK);
}

void LiquidCrystalSerial::init(Pin strobe, Pin data, Pin clk)
{
  _strobe_pin = strobe;
  _data_pin = data;
  _clk_pin = clk;
  
  _strobe_pin.setDirection(true);
  _data_pin.setDirection(true);
  _clk_pin.setDirection(true);
  
  bool fourbitmode = true;
  if (fourbitmode)
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  else 
    _displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
 // begin(16, 1);  
}

void LiquidCrystalSerial::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _numCols = cols;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  _delay_us(50000);
  // Now we pull both RS and R/W low to begin commands
  writeSerial(0b00000000);
  
  //put the LCD into 4 bit or 8 bit mode
  if (! (_displayfunction & LCD_8BITMODE)) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    load(0x03 << 4);
    _delay_us(4500); // wait min 4.1ms

    // second try
    load(0x03 << 4);
    _delay_us(4500); // wait min 4.1ms
    
    // third go!
    load(0x03 << 4); 
    _delay_us(150);

    // finally, set to 8-bit interface
    load(0x02 << 4); 
  } else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    command(LCD_FUNCTIONSET | _displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    command(LCD_FUNCTIONSET | _displayfunction);
    _delay_us(150);

    // third go
    command(LCD_FUNCTIONSET | _displayfunction);
  }

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
    
    // program special characters
    uint8_t right[] = {0,4,2,1,2,4,0};
    uint8_t down[] = {0,0,0,0,0,0x11,0xA,4};
    // write each character twice as sometimes there are signal issues
    createChar(0, right);
    createChar(0, right);
    createChar(1, down);
    createChar(1, down);

}

/********** high level commands, for the user! */
void LiquidCrystalSerial::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

void LiquidCrystalSerial::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

void LiquidCrystalSerial::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row > _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  _xcursor = col; _ycursor = row;
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystalSerial::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystalSerial::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystalSerial::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystalSerial::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystalSerial::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystalSerial::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystalSerial::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystalSerial::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystalSerial::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystalSerial::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystalSerial::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystalSerial::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystalSerial::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystalSerial::command(uint8_t value) {
  send(value, false);
}

inline void LiquidCrystalSerial::write(uint8_t value) {
  send(value, true);
  _xcursor++;
  if(_xcursor >= _numCols)
	setCursor(0,_ycursor+1);
}

void LiquidCrystalSerial::writeInt(uint16_t value, uint8_t digits) {

	uint16_t currentDigit;
	uint16_t nextDigit;

	switch (digits) {
	case 1:		currentDigit = 10;		break;
	case 2:		currentDigit = 100;		break;
	case 3:		currentDigit = 1000;	break;
	case 4:		currentDigit = 10000;	break;
	default: 	return;
	}

	for (uint8_t i = 0; i < digits; i++) {
		nextDigit = currentDigit/10;
		write((value%currentDigit)/nextDigit+'0');
		currentDigit = nextDigit;
	}
}

void LiquidCrystalSerial::writeInt32(uint32_t value, uint8_t digits) {

	uint32_t currentDigit;
	uint32_t nextDigit;

	switch (digits) {
	case 1:		currentDigit = 10;			break;
	case 2:		currentDigit = 100;			break;
	case 3:		currentDigit = 1000;		break;
	case 4:		currentDigit = 10000;		break;
	case 5:		currentDigit = 100000;		break;
	case 6:		currentDigit = 1000000;		break;
	case 7:		currentDigit = 10000000;	break;
	case 8:		currentDigit = 100000000;	break;
	case 9:		currentDigit = 1000000000;	break;
	default: 	return;
	}

	for (uint8_t i = 0; i < digits; i++) {
		nextDigit = currentDigit/10;
		write((value%currentDigit)/nextDigit+'0');
		currentDigit = nextDigit;
	}
}


char* LiquidCrystalSerial::writeLine(char* message) {
	char* letter = message;
	while (*letter != 0 && *letter != '\n') {
		write(*letter);
		letter++;
		
	}
	return letter;
}

void LiquidCrystalSerial::writeString(char message[]) {
	char* letter = message;
	while (*letter != 0) {
		write(*letter);
		letter++;
	}
}

void LiquidCrystalSerial::writeFromPgmspace(const prog_uchar message[]) {
	char letter;
	while (letter = pgm_read_byte(message++)) {
		write(letter);
	}
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystalSerial::send(uint8_t value, bool mode) {
	
	uint8_t modeBits;
	// set mode value
	if(mode)
		modeBits = 0b0010;
	else
		modeBits = 0b0000;
		
 //serial assumes 4 bit mode
    load((value&0xF0) + modeBits);
    load(((value<<4)&0xF0) + modeBits);
}

void LiquidCrystalSerial::load(uint8_t value)
{
	writeSerial(value);
	pulseEnable(value);
}

void LiquidCrystalSerial::pulseEnable(uint8_t value) {
	
  _delay_us(1);
   // set enable to true
   value |= 0b01000;
   writeSerial(value);
  _delay_us(1);    // enable pulse must be >450ns
   //set enable to false
   value &= 0b11110111;
   writeSerial(value);
  _delay_us(1);   // commands need > 37us to settle [citation needed]
}

void LiquidCrystalSerial::writeSerial(uint8_t value) {
  
  int i;
  
  for (i = 7; i >= 0; i--)
  {
  	  _clk_pin.setValue(false);
	  bool data = (value >> i) & 0x01 ? true : false;
	  _data_pin.setValue(data);
	  
	  _clk_pin.setValue(true);
	  _delay_us(1); 
  } 
  
    _strobe_pin.setValue(true);
    _delay_us(1);
    _strobe_pin.setValue(false);
     
}
