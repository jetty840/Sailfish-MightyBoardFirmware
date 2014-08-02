/* LiquidCrystalSerial
 * 
 * This is a base class for control of a HD44780-based LCD display.
 * It should be subclassed to provide specific implementation of the
 * communication routines for specific hardware.
 * 
 * For example, the standard OEM MBI hardware uses a shift register to
 * send data to the LCD display.  Other hardware might use I2C to do
 * accomplish the same thing.
 * 
 * This base class contains the initialization and convenience methods
 * that are similar for all LCD displays.  These methods rely on the
 * subclass' implementaiton of the low level communication routines
 * such as send, writeSerial, write4bits, and pulseEnable.
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

#ifndef LIQUID_CRYSTAL_HH
#define LIQUID_CRYSTAL_HH

// TODO: Proper attribution

#include <stdint.h>
#include <avr/pgmspace.h>
#include "Pin.hh"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// Custom chars
// Unlike the Gen 4 LCD, this module -- ACM2004 series -- this
// LCD display module only provides 8 custom characters and
// n % 8 == n

#define LCD_CUSTOM_CHAR_DOWN 0
#define LCD_CUSTOM_CHAR_EXTRUDER_NORMAL 2
#define LCD_CUSTOM_CHAR_EXTRUDER_HEATING 3
#define LCD_CUSTOM_CHAR_PLATFORM_NORMAL 4
#define LCD_CUSTOM_CHAR_PLATFORM_HEATING 5
#define LCD_CUSTOM_CHAR_FOLDER 6 // Must not be 0
#define LCD_CUSTOM_CHAR_RETURN 7 // Must not be 0

#define LCD_CUSTOM_CHAR_DEGREE 0xdf // MAY ALSO BE 0xdf
#define LCD_CUSTOM_CHAR_UP 0x5e     // ^
#define LCD_CUSTOM_CHAR_RIGHT 0x7e // right pointing arrow (0x7f is left pointing)

// TODO:  make variable names for rs, rw, e places in the output vector

class LiquidCrystalSerial {

public:
  LiquidCrystalSerial();
  
  //LiquidCrystalSerial(Pin strobe, Pin data, Pin CLK) = 0;
  void begin(uint8_t cols, uint8_t rows,
                     uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void homeCursor();      // faster version of home()
  void clearHomeCursor(); // clear() and homeCursor() combined
  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t);
  void setRow(uint8_t);
  void setCursorExt(int8_t col, int8_t row);

  virtual void write(uint8_t);

  /** Added by MakerBot Industries to support storing strings in flash **/
  void writeInt(uint16_t value, uint8_t digits);
  void moveWriteInt(uint8_t col, uint8_t row, uint16_t value, uint8_t digits);
  void writeInt32(uint32_t value, uint8_t digits);
  void writeFloat(float value, uint8_t decimalPlaces,
                  uint8_t rightJustifyToCol);

  void writeString(char message[]);

  /** Display the given line until a newline or null is encountered.
   * Returns a pointer to the first character not displayed.
   */
  char *writeLine(char *message);

  void writeFromPgmspace(const prog_uchar message[]);
  void moveWriteFromPgmspace(uint8_t col, uint8_t row,
                             const prog_uchar message[]);

  void command(uint8_t);

protected:
  /* Sends 8-bits to the HD44780 in two 4-bit transmissions. */  
  virtual void send(uint8_t, bool) = 0;
  
  /* Sets the state of the shift register or bus expander directly
   *
   * Note, it is assumed that the caller knows which bits map where */  
  virtual void writeSerial(uint8_t) = 0;
  
 /* Maps the 4 least-significant bits of value to the proper pins and
  * sends the data to the HD44780 with an enable pulse. */  
  virtual void write4bits(uint8_t value, bool dataMode) = 0;

 /* Sets the state of the shift register or bus expander directly, and
  * then toggles the proper enable line as per the mapping.
  *
  * Note, with the exception of the ENABLE bit to pulse, it is assumed 
  * that the caller knows which bits map where. */  
  virtual void pulseEnable(uint8_t value) = 0;
  
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _xcursor;
  uint8_t _ycursor;

  uint8_t _numlines,_numCols;
  
};

#endif // LIQUID_CRYSTAL_HH
