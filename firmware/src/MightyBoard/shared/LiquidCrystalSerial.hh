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

// TODO:  make variable names for rs, rw, e places in the output vector

class LiquidCrystalSerial {
public:
  LiquidCrystalSerial(Pin strobe, Pin data, Pin CLK);

  void init(Pin strobe, Pin data, Pin CLK);
    
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

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
  virtual void write(uint8_t);

  /** Added by MakerBot Industries to support storing strings in flash **/
  void writeInt(uint16_t value, uint8_t digits);
  void writeInt32(uint32_t value, uint8_t digits);

  void writeString(char message[]);

  /** Display the given line until a newline or null is encountered.
   * Returns a pointer to the first character not displayed.
   */
  char* writeLine(char* message);

  void writeFromPgmspace(const prog_uchar message[]);

  void command(uint8_t);

private:
  void send(uint8_t, bool);
  void writeSerial(uint8_t);
  void load(uint8_t);
  void pulseEnable(uint8_t value);

  Pin _strobe_pin; // LOW: command.  HIGH: character.
  Pin _data_pin; // LOW: write to LCD.  HIGH: read from LCD.
  Pin _clk_pin; // activated by a HIGH pulse.

  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _initialized;

  uint8_t _xcursor;
  uint8_t _ycursor;

  uint8_t _numlines,_numCols;
};

#endif // LIQUID_CRYSTAL_HH
